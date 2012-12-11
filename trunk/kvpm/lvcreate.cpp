/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "lvcreate.h"

#include "fsextend.h"
#include "logvol.h"
#include "misc.h"
#include "mountentry.h"
#include "physvol.h"
#include "pvgroupbox.h"
#include "processprogress.h"
#include "sizeselectorbox.h"
#include "volgroup.h"

#include <math.h>

#include <KComboBox>
#include <KConfigSkeleton>
#include <KGlobal>
#include <KIntSpinBox>
#include <KLineEdit>
#include <KLocale>
#include <KMessageBox>

#include <QCheckBox>
#include <QDebug>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>



/* This class handles both the creation and extension of logical
   volumes and snapshots since both processes are so similar. */


// Creating a new volume

LVCreateDialog::LVCreateDialog(VolGroup *const vg, const bool ispool, QWidget *parent):
    LvCreateDialogBase(vg, -1, false, false, false, ispool, QString(""), QString(""), parent),
    m_ispool(ispool)
{
    m_lv = NULL;
    m_extend = false;
    m_snapshot = false;
    m_bailout  = hasInitialErrors();

    if (!m_bailout)
        buildDialog();
}


// Extending an existing volume or creating a snapshot

LVCreateDialog::LVCreateDialog(LogVol *const volume, const bool snapshot, QWidget *parent):
    LvCreateDialogBase(volume->getVg(), 
                       volume->isCowSnap() ? -1 : fs_max_extend(volume->getMapperPath(), volume->getFilesystem()),
                       !snapshot, snapshot, false, volume->isThinPool(), volume->getName(), 
                       QString(""), parent),

    m_ispool(volume->isThinPool()),
    m_snapshot(snapshot),
    m_extend(!snapshot),
    m_lv(volume)
{
    m_bailout = hasInitialErrors();

    if (!m_bailout)
        buildDialog();
}

void LVCreateDialog::buildDialog()
{
    VolGroup *const vg = getVg();

    if (m_extend)
        initializeSizeSelector(vg->getExtentSize(), m_lv->getExtents(), vg->getAllocatableExtents() + m_lv->getExtents());
    else
        initializeSizeSelector(vg->getExtentSize(), 0, vg->getAllocatableExtents());
        
    m_physical_tab = createPhysicalTab();
    setPhysicalTab(m_physical_tab);
    
    enableTypeOptions(m_type_combo->currentIndex());
    enableStripeCombo(m_stripe_count_spin->value());
    makeConnections();
    setMaxSize();
    resetOkButton();

    if (!m_snapshot && !m_extend && !m_ispool)
        setZero(true);
    else if (m_ispool)
        setZero(true);
}

void LVCreateDialog::makeConnections()
{
    connect(m_pv_box, SIGNAL(stateChanged()),
            this, SLOT(setMaxSize()));

    connect(m_stripe_count_spin, SIGNAL(valueChanged(int)),
            this, SLOT(enableStripeCombo(int)));

    connect(m_stripe_count_spin, SIGNAL(valueChanged(int)),
            this, SLOT(setMaxSize()));

    connect(m_mirror_count_spin, SIGNAL(valueChanged(int)),
            this, SLOT(setMaxSize()));

    connect(m_type_combo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(enableTypeOptions(int)));

    connect(m_type_combo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setMaxSize()));

    connect(m_type_combo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(enableMonitoring(int)));

    connect(m_log_combo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setMaxSize()));

    connect(this, SIGNAL(extendFs()),
            this, SLOT(setMaxSize()));
}


QWidget* LVCreateDialog::createPhysicalTab()
{
    QString message;

    QVBoxLayout *const layout = new QVBoxLayout;
    m_physical_tab = new QWidget(this);
    m_physical_tab->setLayout(layout);

    QList<PhysVol *> pvs(getVg()->getPhysicalVolumes());

    for (int x = pvs.size() - 1; x >= 0; x--) {
        if (pvs[x]->getRemaining() < 1 || !pvs[x]->isAllocatable() || pvs[x]->isMissing())
            pvs.removeAt(x);
    }

    QList<long long> normal;
    QList<long long> contiguous;

    if (m_lv != NULL) {

        for (int x = 0; x < pvs.size(); x++) {
            normal.append(pvs[x]->getRemaining());
            contiguous.append(pvs[x]->getContiguous(m_lv));
        }

        m_pv_box = new PvGroupBox(pvs, normal, contiguous, m_lv->getPolicy());
    } else {

        for (int x = 0; x < pvs.size(); x++) {
            normal.append(pvs[x]->getRemaining());
            contiguous.append(pvs[x]->getContiguous(m_lv));
        }

        m_pv_box = new PvGroupBox(pvs, normal, contiguous, INHERITED);
    }

    layout->addWidget(m_pv_box);

    QHBoxLayout *const lower_h_layout = new QHBoxLayout;
    QVBoxLayout *const lower_v_layout = new QVBoxLayout;
    layout->addLayout(lower_h_layout);
    lower_h_layout->addStretch();
    lower_h_layout->addLayout(lower_v_layout);
    lower_h_layout->addStretch();

    m_volume_box = new QGroupBox();
    QVBoxLayout *const volume_layout = new QVBoxLayout;
    m_volume_box->setLayout(volume_layout);

    volume_layout->addWidget(createTypeWidget(pvs.size()));
    m_stripe_widget = createStripeWidget();
    m_mirror_widget = createMirrorWidget(pvs.size());

    if (m_ispool && !m_extend) {
        volume_layout->addWidget(createChunkWidget());
        connect(m_chunk_combo, SIGNAL(currentIndexChanged(int)),
                this, SLOT(setMaxSize()));
    }

    volume_layout->addWidget(m_stripe_widget);
    volume_layout->addWidget(m_mirror_widget);
    volume_layout->addStretch();

    lower_v_layout->addWidget(m_volume_box);

    return m_physical_tab;
}

int LVCreateDialog::getChunkSize()  // returns pool chunk size in bytes
{
    int chunk = 0x10000; // 64KiB

    if (m_extend) {
        chunk = m_lv->getChunkSize(0);  // if chunk size can be different across segments this will need to be changed
    } else if (m_chunk_combo->currentIndex() > 0) {
        chunk = QVariant(m_chunk_combo->itemData(m_chunk_combo->currentIndex(), Qt::UserRole)).toInt();
    } else {
        long long meta = (64 * getSelectorExtents() * getVg()->getExtentSize()) / chunk;

        while ((meta > 0x8000000) && (chunk < 0x40000000)) {  // meta > 128MiB and chunk < 1GB
            chunk *= 2;
            meta /= 2;
        }
    }

    return chunk;
}

int LVCreateDialog::getChunkSize(long long volumeSize)  // returns pool chunk size in bytes
{
    int chunk = 0x10000; // 64KiB

    if (m_extend) {
        chunk = m_lv->getChunkSize(0);  // if chunk size can be different across segments this will need to be changed
    } else if (m_chunk_combo->currentIndex() > 0) {
        chunk = QVariant(m_chunk_combo->itemData(m_chunk_combo->currentIndex(), Qt::UserRole)).toInt();
    } else {
        long long meta = (64 * volumeSize) / chunk;

        while ((meta > 0x8000000) && (chunk < 0x40000000)) {  // meta > 128MiB and chunk < 1GB
            chunk *= 2;
            meta /= 2;
        }
    }

    return chunk;
}

QWidget* LVCreateDialog::createChunkWidget()
{
    QWidget *const widget = new QWidget;
    QHBoxLayout *const layout = new QHBoxLayout;
    layout->addWidget(new QLabel(i18n("Chunk size: ")));
    m_chunk_combo = new KComboBox();
    m_chunk_combo->addItem(i18n("default"));
    layout->addWidget(m_chunk_combo);    
    unsigned int chunk;

    for (int n = 0; n < 15; n++) {
        chunk = round(64 * pow(2, n));

        if (chunk < 1000)
            m_chunk_combo->addItem(QString("%1").arg(chunk) + " KiB");
        else 
            m_chunk_combo->addItem(QString("%1").arg(chunk/1024) + " MiB");

        m_chunk_combo->setItemData(n + 1, QVariant(chunk * 1024), Qt::UserRole);  // chunk size in bytes
    }

    widget->setLayout(layout);
    return widget;
}    

QWidget* LVCreateDialog::createStripeWidget()
{
    QWidget *const widget = new QWidget;
    QVBoxLayout *const layout = new QVBoxLayout;

    m_stripe_size_combo = new KComboBox();
    m_stripe_size_combo->setEnabled(false);
    for (int n = 2; (round(pow(2, n) * 1024)) <= getVg()->getExtentSize() ; n++) {
        m_stripe_size_combo->addItem(QString("%1").arg(round(pow(2, n))) + " KiB");
        m_stripe_size_combo->setItemData(n - 2, QVariant(round(pow(2, n))), Qt::UserRole);
        m_stripe_size_combo->setEnabled(true);   // only enabled if the combo box has at least one entry!
        
        if ((n - 2) < 5) 
            m_stripe_size_combo->setCurrentIndex(n - 2);
    }
    
    QLabel *const size_label = new QLabel(i18n("Stripe Size: "));
    size_label->setBuddy(m_stripe_size_combo);
    
    m_stripe_count_spin = new KIntSpinBox();
    m_stripe_count_spin->setEnabled(m_stripe_size_combo->isEnabled());
    m_stripe_count_spin->setRange(1, getMaxStripes()); 
    m_stripe_count_spin->setSpecialValueText(i18n("none"));

    if (m_stripe_size_combo->isEnabled()) {
        QHBoxLayout *const size_layout = new QHBoxLayout;
        size_layout->addWidget(size_label);
        size_layout->addWidget(m_stripe_size_combo);
        QLabel *const count_label = new QLabel(i18n("Number of stripes: "));
        count_label->setBuddy(m_stripe_count_spin);
        QHBoxLayout *const count_layout = new QHBoxLayout;
        count_layout->addWidget(count_label);
        count_layout->addWidget(m_stripe_count_spin);
        
        layout->addLayout(size_layout);
        layout->addLayout(count_layout);
    } else {
        QLabel *const error_label = new QLabel(i18n("Extents smaller than 4KiB can not be striped"));
        layout->addWidget(error_label);

        for (int x = m_type_combo->count() - 1; x > 2; x--)
            m_type_combo->removeItem(x);
    }

    widget->setLayout(layout);

    /* If we are extending a volume we try to match the striping
       of the last segment of that volume, if it was striped */

    if (m_extend) {
        int seg_count = 1;
        int stripe_count = 1;
        int stripe_size = 4;
        QList<LogVol *>  logvols;

        if (m_lv->isLvmMirror()) {                        // Tries to match striping to last segment of first leg
            logvols = m_lv->getAllChildrenFlat();
            for (int x = 0; x < logvols.size(); x++) {
                if (logvols[x]->isLvmMirrorLeg() && !(logvols[x]->isLvmMirrorLog())) {
                    seg_count = logvols[x]->getSegmentCount();
                    stripe_count = logvols[x]->getSegmentStripes(seg_count - 1);
                    stripe_size = logvols[x]->getSegmentStripeSize(seg_count - 1);
                    break;
                }
            }
        } else {
            seg_count = m_lv->getSegmentCount();
            stripe_count = m_lv->getSegmentStripes(seg_count - 1);
            stripe_size = m_lv->getSegmentStripeSize(seg_count - 1);

            if (m_lv->isRaid()) { 
                m_stripe_count_spin->setEnabled(false);

                if (m_lv->getRaidType() == 4 || m_lv->getRaidType() == 5){
                    stripe_count -= 1;
                    m_stripe_count_spin->setSuffix(i18n(" + 1 parity")); 
                    m_stripe_count_spin->setSpecialValueText("");
                } else if (m_lv->getRaidType() == 6) {
                    stripe_count -= 2;
                    m_stripe_count_spin->setSuffix(i18n(" + 2 parity"));
                    m_stripe_count_spin->setSpecialValueText("");
                }
            }
        }

        m_stripe_count_spin->setValue(stripe_count);

        if (stripe_count > 1) {
            int stripe_index = m_stripe_size_combo->findData(QVariant(stripe_size / 1024));

            if (stripe_index == -1)
                stripe_index = 0;
            m_stripe_size_combo->setCurrentIndex(stripe_index);
        }
    }

    return widget;
}

QWidget* LVCreateDialog::createMirrorWidget(int pvcount)
{
    QWidget *const widget = new QWidget;

    m_log_combo = new KComboBox;
    m_log_combo->addItem(i18n("Mirrored disk based log"));
    m_log_combo->addItem(i18n("Disk based log"));
    m_log_combo->addItem(i18n("Memory based log"));

    QHBoxLayout *const log_layout = new QHBoxLayout();
    QLabel *const log_label  = new QLabel(i18n("Mirror log: "));
    log_label->setBuddy(m_log_combo);
    log_layout->addWidget(log_label);
    log_layout->addWidget(m_log_combo);

    QHBoxLayout *const spin_layout = new QHBoxLayout();
    m_mirror_count_spin = new KIntSpinBox();
    m_mirror_count_spin->setRange(1, pvcount);

    if (m_extend){
        if (m_lv->isLvmMirror() || m_lv->getRaidType() == 1)
            m_mirror_count_spin->setValue(m_lv->getMirrorCount());
        else
            m_mirror_count_spin->setValue(1);

        if (m_lv->isLvmMirror()) {
            if (m_lv->getLogCount() == 0)
                m_log_combo->setCurrentIndex(2);
            else if (m_lv->getLogCount() == 1)
                m_log_combo->setCurrentIndex(1);
            else if (m_lv->getLogCount() == 2)
                m_log_combo->setCurrentIndex(0);
        }
        else
            m_log_combo->setCurrentIndex(1);

        m_log_combo->setEnabled(false);
        m_mirror_count_spin->setEnabled(false);
    }

    QLabel *const count_label  = new QLabel(i18n("Number of mirror legs: "));
    count_label->setBuddy(m_mirror_count_spin);
    spin_layout->addWidget(count_label);
    spin_layout->addWidget(m_mirror_count_spin);

    QVBoxLayout *const layout = new QVBoxLayout;
    layout->addLayout(log_layout);
    layout->addLayout(spin_layout);
    widget->setLayout(layout);

    return widget;
}

QWidget* LVCreateDialog::createTypeWidget(int pvcount)
{
    QWidget *const widget = new QWidget;

    m_type_combo = new KComboBox();
    m_type_combo->addItem(i18n("Linear"));

    if (m_extend) {
        const bool ismirror = m_lv->isMirror();
        const bool israid   = m_lv->isRaid();
        const int raidtype  = m_lv->getRaidType();

        m_type_combo->setEnabled(false);
        m_type_combo->addItem(i18n("LVM2 Mirror"));
        m_type_combo->addItem(i18n("RAID 1 Mirror"));
        m_type_combo->addItem(i18n("RAID 4"));
        m_type_combo->addItem(i18n("RAID 5"));
        m_type_combo->addItem(i18n("RAID 6"));

        if (ismirror && !israid) {
            m_type_combo->setCurrentIndex(1);
        } else if (israid){
            if (raidtype == 1)
                m_type_combo->setCurrentIndex(2);
            else if (raidtype == 4)
                m_type_combo->setCurrentIndex(3);
            else if (raidtype == 5)
                m_type_combo->setCurrentIndex(4);
            else if (raidtype == 6)
                m_type_combo->setCurrentIndex(5);
            else
                m_type_combo->setCurrentIndex(0);
        } else {
            m_type_combo->setCurrentIndex(0);
        }
    } else if (!m_ispool) {
        
        if (pvcount > 1) {
            m_type_combo->addItem(i18n("LVM2 Mirror"));
            m_type_combo->addItem(i18n("RAID 1 Mirror"));
        }
        
        if (pvcount > 2) {
            m_type_combo->addItem(i18n("RAID 4"));
            m_type_combo->addItem(i18n("RAID 5"));
        }
        
        if (pvcount > 4) {
            m_type_combo->addItem(i18n("RAID 6"));
        }
    }

    QLabel *const type_label = new QLabel(i18n("Volume type: "));
    type_label->setBuddy(m_type_combo);

    if (m_snapshot) {
        m_type_combo->setCurrentIndex(0);
        m_type_combo->setEnabled(false);
        type_label->hide();
        m_type_combo->hide();
    } else if (m_extend) {
        m_type_combo->setEnabled(false);
        type_label->hide();
        m_type_combo->hide();
    }

    QHBoxLayout *const layout = new QHBoxLayout;
    layout->addWidget(type_label);
    layout->addWidget(m_type_combo);

    widget->setLayout(layout);

    return widget;
}

void LVCreateDialog::setMaxSize()
{
    VolumeType type;
    const int stripes = m_stripe_count_spin->value();
    const int mirrors = m_mirror_count_spin->value();

    m_stripe_count_spin->setMaximum(getMaxStripes());

    const long long max = getLargestVolume() / getVg()->getExtentSize();
    const long long maxfs = getMaxFsSize() / getVg()->getExtentSize();

    if (getExtendFs()) {
        if (max < maxfs)
            setSelectorMaxExtents(max);
        else
            setSelectorMaxExtents(maxfs);
    } else {
        setSelectorMaxExtents(max);
    }

    resetOkButton();

    switch (m_type_combo->currentIndex()) {
    case 0:
        type = LINEAR;
        break;
    case 1:
        type = LVMMIRROR;
        break;
    case 2:
        type = RAID1;
        break;
    case 3:
        type = RAID4;
        break;
    case 4:
        type = RAID5;
        break;
    case 5:
        type = RAID6;
        break;
    default:
        type = LINEAR;
        break;
    }

    setInfoLabels(type, stripes, mirrors, getLargestVolume());
}

void LVCreateDialog::resetOkButton()
{
    const long long max = getLargestVolume() / getVg()->getExtentSize();
    const long long selected = getSelectorExtents();
    const long long rounded  = roundExtentsToStripes(selected);

    if (!LvCreateDialogBase::isValid()) {
        if (rounded > max) {
            setWarningLabel("Selected size exceeds maximum size");
        } else if (m_extend) {
            if (isLow())
                setWarningLabel(i18n("Selected size less than existing size"));
            else
                clearWarningLabel();
        } else {
            clearWarningLabel();
        }

        enableButtonOk(false);
    } else {
        if (m_extend) {
            if ((rounded <= max) && (rounded > m_lv->getExtents()) && (selected > m_lv->getExtents())) {
                clearWarningLabel();
                enableButtonOk(true);
            } else {
                setWarningLabel(i18n("Selected size exceeds maximum size"));
                enableButtonOk(false);
            }
        } else {
            if ((rounded <= max) && (rounded > 0)) {
                clearWarningLabel();
                enableButtonOk(true);
            } else {
                setWarningLabel(i18n("Selected size exceeds maximum size"));
                enableButtonOk(false);
            }
        }
    }
}

void LVCreateDialog::enableTypeOptions(int index)
{
    const int pv_count = m_pv_box->getAllNames().size();

    if (index == 0) {  // linear
        m_mirror_count_spin->setMinimum(1);
        m_mirror_count_spin->setValue(1);
        m_mirror_count_spin->setSpecialValueText(i18n("none"));
        m_mirror_count_spin->setEnabled(false);
        m_log_combo->setEnabled(false);
        m_log_combo->setCurrentIndex(1);
        m_stripe_count_spin->setRange(1, getMaxStripes());
        if(!m_extend)
            m_stripe_count_spin->setValue(1);
        m_stripe_count_spin->setSuffix("");
        m_stripe_count_spin->setSpecialValueText(i18n("none"));
        m_stripe_count_spin->setEnabled(true);

        m_stripe_widget->show();
        m_mirror_widget->hide();

    } else if (index == 1) {  // LVM2 mirror

        if(m_extend) {
            m_mirror_count_spin->setEnabled(false);
            m_log_combo->setEnabled(false);
        } else {
            m_mirror_count_spin->setEnabled(true);
            m_log_combo->setEnabled(true);
            m_mirror_count_spin->setMinimum(2);
            m_mirror_count_spin->setValue(2);
            m_mirror_count_spin->setMaximum(pv_count);
            m_mirror_count_spin->setSpecialValueText("");
            m_log_combo->setCurrentIndex(1);
            m_stripe_count_spin->setMinimum(1);
            m_stripe_count_spin->setValue(1);
            m_stripe_count_spin->setMaximum(getMaxStripes());
            m_stripe_count_spin->setSuffix("");
            m_stripe_count_spin->setSpecialValueText(i18n("none"));
        }

        m_stripe_count_spin->setEnabled(true);
        m_stripe_widget->show();
        m_mirror_widget->show();
    } else if (index == 2) {  // RAID 1

        if(m_extend)
            m_mirror_count_spin->setEnabled(false);
        else {
            m_mirror_count_spin->setMinimum(2);
            m_mirror_count_spin->setValue(2);
            m_mirror_count_spin->setMaximum(pv_count);
            m_mirror_count_spin->setSpecialValueText("");
            m_mirror_count_spin->setEnabled(true);
        }

        m_log_combo->setEnabled(false);
        m_log_combo->setCurrentIndex(1);
        m_stripe_count_spin->setMinimum(1);
        m_stripe_count_spin->setValue(1);
        m_stripe_count_spin->setSuffix("");
        m_stripe_count_spin->setSpecialValueText(i18n("none"));
        m_stripe_count_spin->setEnabled(false);
        m_stripe_size_combo->setEnabled(false);
        m_stripe_widget->hide();
        m_mirror_widget->show();
    } else if (index == 3) {  // RAID 4
        m_mirror_count_spin->setMinimum(1);
        m_mirror_count_spin->setValue(1);
        m_mirror_count_spin->setSpecialValueText(i18n("none"));
        m_mirror_count_spin->setEnabled(false);
        m_log_combo->setEnabled(false);
        m_log_combo->setCurrentIndex(1);

        if(m_extend) {
            m_stripe_count_spin->setEnabled(false);
        } else{
            m_stripe_count_spin->setMinimum(2);
            m_stripe_count_spin->setValue(2);
            m_stripe_count_spin->setMaximum(getMaxStripes() - 1);
            m_stripe_count_spin->setSpecialValueText("");
            m_stripe_count_spin->setSuffix(i18n(" + 1 parity"));
            m_stripe_count_spin->setEnabled(true);
        }

        m_stripe_widget->show();
        m_mirror_widget->hide();

    } else if (index == 4) {  // RAID 5
        m_mirror_count_spin->setMinimum(1);
        m_mirror_count_spin->setValue(1);
        m_mirror_count_spin->setSpecialValueText(i18n("none"));
        m_mirror_count_spin->setEnabled(false);
        m_log_combo->setEnabled(false);
        m_log_combo->setCurrentIndex(1);

        if(m_extend) {
            m_stripe_count_spin->setEnabled(false);
        } else {
            m_stripe_count_spin->setMinimum(2);
            m_stripe_count_spin->setValue(2);
            m_stripe_count_spin->setMaximum(getMaxStripes() - 1);
            m_stripe_count_spin->setSpecialValueText("");
            m_stripe_count_spin->setSuffix(i18n(" + 1 parity"));
            m_stripe_count_spin->setEnabled(true);
        }

        m_stripe_widget->show();
        m_mirror_widget->hide();
    } else if (index == 5) {  // RAID 6
        m_mirror_count_spin->setMinimum(1);
        m_mirror_count_spin->setValue(1);
        m_mirror_count_spin->setSpecialValueText(i18n("none"));
        m_mirror_count_spin->setEnabled(false);
        m_log_combo->setEnabled(false);
        m_log_combo->setCurrentIndex(1);

        if(m_extend) {
            m_stripe_count_spin->setEnabled(false);
        } else {
            m_stripe_count_spin->setMinimum(3);
            m_stripe_count_spin->setValue(3); 
            m_stripe_count_spin->setMaximum(getMaxStripes() - 2);
            m_stripe_count_spin->setSpecialValueText("");
            m_stripe_count_spin->setSuffix(i18n(" + 2 parity"));
            m_stripe_count_spin->setEnabled(true);
        }

        m_stripe_widget->show();
        m_mirror_widget->hide();
    }
}

void LVCreateDialog::enableStripeCombo(int value)
{
    if (value > 1 && m_stripe_size_combo->count() > 0)
        m_stripe_size_combo->setEnabled(true);
    else
        m_stripe_size_combo->setEnabled(false);
}

void LVCreateDialog::enableMonitoring(int index)
{
    if (index == 1 || index == 2) {          // lvm mirror or raid mirror
        setMonitor(true);                    // whether a snap or not
        enableMonitor(true);
        enableSkipSync(true);
    } else if (index > 2 || m_snapshot){      // raid stripe set
        setMonitor(true);                     // and all other snaps
        enableMonitor(true);
        setSkipSync(false);
        enableSkipSync(false);
    } else {                                  // linear
        setMonitor(false);
        enableMonitor(false);
        setSkipSync(false);
        enableSkipSync(false);
    }
}

/* largest volume that can be created given the pvs, striping and mirrors
   selected. This includes the size of the already existing volume if we
   are extending a volume */

long long LVCreateDialog::getLargestVolume()
{
    int total_stripes;
    const int type = m_type_combo->currentIndex();
    const int stripe_count = m_stripe_count_spin->value();
    const int mirror_count = m_mirror_count_spin->value();
    const long long extent_size = getVg()->getExtentSize();

    if (type == 1)          // LVM2 mirror
        total_stripes = stripe_count * mirror_count;
    else if (type == 2)    // RAID 1 mirror
        total_stripes = mirror_count;
    else
        total_stripes = stripe_count;

    if (type == 3)         // RAID 4
        total_stripes += 1;
    else if (type == 4)    // RAID 5
        total_stripes += 1;
    else if (type == 5)    // RAID 6
        total_stripes += 2;

    int log_count = 0;

    if (type == 1 && !m_extend) {
        if (m_log_combo->currentIndex() == 0)
            log_count = 2;
        else if (m_log_combo->currentIndex() == 1)
            log_count = 1;
        else
            log_count = 0;
    } else {
        log_count = 0;
    }

    AllocationPolicy policy = m_pv_box->getPolicy();
    if (policy == INHERITED)
        policy = getVg()->getPolicy();

    QList <long long> stripe_pv_bytes;
    for (int x = 0; x < total_stripes; x++)
        stripe_pv_bytes.append(0);

    QList <long long> available_pv_bytes = m_pv_box->getRemainingSpaceList();

    if (!m_extend) {
        qSort(available_pv_bytes);
        if (policy == CONTIGUOUS) {
            while (available_pv_bytes.size() > total_stripes + log_count)  
                available_pv_bytes.removeFirst();
        } 

        for (int x = 0; x < log_count; x++) {
            if (available_pv_bytes.size())
                available_pv_bytes.removeFirst();
            else
                return 0;
        }

    } else {
        extendLastSegment(stripe_pv_bytes, available_pv_bytes);
    }

    while (available_pv_bytes.size()) {
        qSort(stripe_pv_bytes);
        stripe_pv_bytes[0] += available_pv_bytes.takeLast();
    }
    qSort(stripe_pv_bytes);

// The next function sets aside the space needed for thin pool metadata.
// The data doesn't grow upon extending of the pool.

    if (m_ispool && !m_extend) {  

        m_ispool = false;
        long long meta = 64 * (getLargestVolume() / getChunkSize(getLargestVolume()));
        m_ispool = true;
        const long long ext = getVg()->getExtentSize();
        meta = ( (meta  + ext - 1 ) / ext) * ext;

        if (meta < 0x200000)   // 2 MiB 
            meta = 0x200000;  
        else if (meta > 0x400000000)   // 16 GiB 
            meta = 0x400000000;
                        
        long long longest = stripe_pv_bytes.takeLast() - meta;
        if (longest < 0)
            longest = 0;
        stripe_pv_bytes.append(longest);
        qSort(stripe_pv_bytes);
    }

    long long largest = stripe_pv_bytes[0];

    if (!m_extend) {
        if (type == 2)
            largest = largest - extent_size; // RAID 1 uses one extent per mirror for metadata
        else if (type == 3 || type == 4 || type == 5) 
            largest = (largest - extent_size) * stripe_count; // RAID 4/5/6 use one per stripe
        else 
            largest = largest * stripe_count;
    } else {
        if (m_lv->isMirror() && policy == CONTIGUOUS) {

            const QList<LogVol *> legs = m_lv->getChildren(); // not grandchildren because we can't extend while under conversion
            QList<long long> leg_max;

            for (int x = legs.size() - 1; x >= 0; x--) {
                if (legs[x]->isMirrorLeg()) {

                    const QStringList pv_names = legs[x]->getPvNames(legs[x]->getSegmentCount() - 1);
                    QList<long long> stripe_max;
                  
                    for (int y = pv_names.size() - 1; y >= 0; y--)
                        stripe_max.append(getVg()->getPvByName(pv_names[y])->getContiguous(m_lv));

                    qSort(stripe_max);

                    if (stripe_max.size() < stripe_count)
                        return m_lv->getSize();

                    while (stripe_max.size() > stripe_count)
                        stripe_max.removeFirst();

                    leg_max.append(stripe_max[0] * stripe_count);
                }
            }

            qSort(leg_max);
            largest = leg_max[0] + m_lv->getSize();

        } else if (policy == CONTIGUOUS) {

            const QStringList pv_names = m_lv->getPvNames(m_lv->getSegmentCount() - 1);
            QList<long long> stripe_max;
            
            for (int y = pv_names.size() - 1; y >= 0; y--)
                stripe_max.append(getVg()->getPvByName(pv_names[y])->getContiguous(m_lv));
            
            qSort(stripe_max);
            
            if (stripe_max.size() < stripe_count)
                return m_lv->getSize();
            
            while (stripe_max.size() > stripe_count)
                stripe_max.removeFirst();

            largest = (stripe_max[0] * stripe_count) + m_lv->getSize();

        } else {
            largest = (largest * stripe_count) + m_lv->getSize();
        }
    }

    if (largest < 0)
        largest = 0;

    return largest;
}


/* Here we create a stringlist of arguments based on all
   the options that the user chose in the dialog. */

QStringList LVCreateDialog::args()
{
    QString program_to_run;
    QStringList args;
    const QVariant stripe_size = m_stripe_size_combo->itemData(m_stripe_size_combo->currentIndex(), Qt::UserRole);
    const int stripes = m_stripe_count_spin->value();
    const int type    = m_type_combo->currentIndex();
    const int mirrors = m_mirror_count_spin->value();
    long long extents = getSelectorExtents();

    if (!m_extend) {
        if (!getTag().isEmpty())
            args << "--addtag" << getTag();

        if (getPersistent()) {
            args << "--persistent" << "y";
            args << "--major" << getMajor();
            args << "--minor" << getMinor();
        }

        if (!m_ispool) {
            if (getReadOnly())
                args << "--permission" << "r" ;
            else
                args << "--permission" << "rw" ;
        }

        if (type == 1)
            args << "--type" << "mirror" ;
        else if (type == 2)
            args << "--type" << "raid1" ;
        else if (type == 3)
            args << "--type" << "raid4" ;
        else if (type == 4)
            args << "--type" << "raid5" ;
        else if (type == 5)
            args << "--type" << "raid6" ;

        if (!m_snapshot && !m_extend) {
            if (getZero())
                args << "--zero" << "y";
            else
                args << "--zero" << "n";

            if (mirrors > 1) {
                args << "--mirrors" << QString("%1").arg(mirrors - 1);

                if (getSkipSync())
                    args << "--nosync";

                if (type == 1) { // traditional mirror
                    if (m_log_combo->currentIndex() == 0)
                        args << "--mirrorlog" << "mirrored";
                    else if (m_log_combo->currentIndex() == 1)
                        args << "--mirrorlog" << "disk";
                    else
                        args << "--mirrorlog" << "core";
                }
            } 
        }
    }

    if (!getUdev())
        args << "--noudevsync";

    if (stripes > 1) {
        args << "--stripes" << QString("%1").arg(stripes);
        args << "--stripesize" << QString("%1").arg(stripe_size.toLongLong());
    } else if (m_extend && (type == 0 || type == 1)) {
        args << "--stripes" << QString("%1").arg(1);
    }

    if (type > 0 && !m_extend) {
        args << "--monitor";
        if (getMonitor())
            args << "y";
        else
            args << "n";
    }

    const QString policy = policyToString(m_pv_box->getPolicy());

    if (m_pv_box->getPolicy() != INHERITED)  // "inherited" is what we get if we don't pass "--alloc" at all
        args << "--alloc" << policy;         // passing "--alloc" "inherited" won't work

    if (m_extend)
        extents -= m_lv->getExtents();

    extents = roundExtentsToStripes(extents);

    args << "--extents" << QString("+%1").arg(extents);

    if (m_ispool && !m_extend) {                        // create a thin pool
        program_to_run = "lvcreate";
        args << "--chunksize" << QString("%1k").arg(getChunkSize()/1024);
        args << "--thinpool" << getName();
        args << getVg()->getName();
    } else if (!m_extend && !m_snapshot) {              // create a standard volume
        program_to_run = "lvcreate";

        if (!getName().isEmpty())
            args << "--name" << getName();

        args << getVg()->getName();
    } else if (m_snapshot) {                            // create a snapshot
        program_to_run = "lvcreate";

        args << "--snapshot";

        if (!getName().isEmpty())
            args << "--name" << getName();

        args << m_lv->getFullName();
    } else {                                            // extend the current volume
        program_to_run = "lvextend";
        args << m_lv->getFullName();
    }

    args << m_pv_box->getNames();
    args.prepend(program_to_run);

    return args;
}

// make the number of extents divivsible by the stripe X mirror count then round up

long long LVCreateDialog::roundExtentsToStripes(long long extents)
{
    const int stripes = m_stripe_count_spin->value();
    const int mirrors = m_mirror_count_spin->value();
    long long max_extents;

    if (m_extend)
        max_extents = (getLargestVolume() - m_lv->getSize()) / getVg()->getExtentSize();
    else
        max_extents = getLargestVolume() / getVg()->getExtentSize();

    // The next part should only need to reference stripes, not the mirror count
    // but a bug in lvm requires it. Remove this when fixed.

    if (stripes > 1) {

        if (extents % (stripes * mirrors)) {

            extents = extents / (stripes * mirrors);
            extents = extents * (stripes * mirrors);

            if (extents + (stripes * mirrors) <= max_extents) {
                extents += (stripes * mirrors);
            }
        }
    }

    return extents;
}






// This function checks for problems that would make showing this dialog pointless
// returns true if there are problems and is used to set m_bailout.

bool LVCreateDialog::hasInitialErrors()
{
    VolGroup *const vg = getVg();

    if (vg->getAllocatableExtents() == 0 || vg->isPartial()) {
        if (vg->isPartial())
            if (m_extend)
                KMessageBox::error(this, i18n("Volumes can not be extended while physical volumes are missing"));
            else
                KMessageBox::error(this, i18n("Volumes can not be created while physical volumes are missing"));
        else if (vg->getFreeExtents())
            KMessageBox::error(this, i18n("All free physical volume extents in this group"
                                          " are locked against allocation"));
        else
            KMessageBox::error(this, i18n("There are no free extents in this volume group"));

        return true;
    }

    if (m_extend) {


        const QString warning1 = i18n("If this volume has a filesystem or data, it will need to be extended later "
                                      "by an appropriate tool. \n \n"
                                      "Currently, only the ext2, ext3, ext4, xfs, jfs, ntfs and reiserfs file systems are "
                                      "supported for extension. ");

        const QString warning2 = i18n("This filesystem seems to be as large as it can get, it will not be extended with the volume");


        if (m_lv->isRaid() || m_lv->isLvmMirror()){
            QList<PhysVol *> pvs = vg->getPhysicalVolumes();

            for (int x = pvs.size() - 1; x >= 0; x--) {
                if (pvs[x]->getRemaining() < 1 || !pvs[x]->isAllocatable())
                    pvs.removeAt(x);
            }

            if (m_lv->getMirrorCount() > pvs.size() ||  (m_lv->isRaid() && m_lv->getSegmentStripes(0) > pvs.size())) {
                KMessageBox::error(this, i18n("Insufficient allocatable physical volumes to extend this volume"));
                return true;
            }
        }

        if (m_lv->isCowOrigin()) {
            if (m_lv->isOpen()) {
                KMessageBox::error(this, i18n("Snapshot origins cannot be extended while open or mounted"));
                return true;
            }

            const QList<LogVol *> snap_shots = m_lv->getSnapshots();

            for (int x = 0; x < snap_shots.size(); x++) {
                if (snap_shots[x]->isOpen()) {
                    KMessageBox::error(this, i18n("Volumes cannot be extended with open or mounted snapshots"));
                    return true;
                }
            }
        }
        
        const long long maxfs = getMaxFsSize() / m_lv->getVg()->getExtentSize();
        const long long current = m_lv->getExtents(); 

        if (!(m_lv->isThinPool() || fs_can_extend(m_lv->getFilesystem()) || m_lv->isCowSnap())) {
            if (KMessageBox::warningContinueCancel(NULL,
                                                   warning1,
                                                   QString(),
                                                   KStandardGuiItem::cont(),
                                                   KStandardGuiItem::cancel(),
                                                   QString(),
                                                   KMessageBox::Dangerous) != KMessageBox::Continue) {
                return true;
            }
        } else if (current >= maxfs) {
            if (KMessageBox::warningContinueCancel(NULL,
                                                   warning2,
                                                   QString(),
                                                   KStandardGuiItem::cont(),
                                                   KStandardGuiItem::cancel(),
                                                   QString(),
                                                   KMessageBox::Dangerous) != KMessageBox::Continue) {
                return true;
            }
        }
    }
    
    return false;
}

bool LVCreateDialog::bailout()
{
    return m_bailout;
}

void LVCreateDialog::commit()
{
    QStringList lvchange_args;
    hide();

    if (!m_extend) {
        ProcessProgress create_lv(args());
        return;
    } else {
        const QString mapper_path = m_lv->getMapperPath();
        const QString fs = m_lv->getFilesystem();

        if (m_lv->isCowOrigin()) {

            lvchange_args << "lvchange" << "-an" << mapper_path;
            ProcessProgress deactivate_lv(lvchange_args);
            if (deactivate_lv.exitCode()) {
                KMessageBox::error(0, i18n("Volume deactivation failed, volume not extended"));
                return;
            }

            ProcessProgress extend_origin(args());
            if (extend_origin.exitCode()) {
                KMessageBox::error(0, i18n("Volume extension failed"));
                return;
            }

            lvchange_args.clear();
            lvchange_args << "lvchange" << "-ay" << mapper_path;
            ProcessProgress activate_lv(lvchange_args);
            if (activate_lv.exitCode()) {
                if (getExtendFs())
                    KMessageBox::error(0, i18n("Volume activation failed, filesystem not extended"));
                else
                    KMessageBox::error(0, i18n("Volume activation failed"));
            } else if (getExtendFs()) {
                fs_extend(m_lv->getMapperPath(), fs, m_lv->getMountPoints(), true);
            }

            return;
        } else {
            ProcessProgress extend_lv(args());
            if (!extend_lv.exitCode() && !m_lv->isCowSnap() && getExtendFs())
                fs_extend(mapper_path, fs, m_lv->getMountPoints(), true);

            return;
        }
    }
}

int LVCreateDialog::getMaxStripes()
{
    int stripes = m_pv_box->getAllNames().size();
    AllocationPolicy policy = m_pv_box->getPolicy();

    if (policy == INHERITED)
        policy = getVg()->getPolicy();

    if (m_extend && (policy == CONTIGUOUS)){
        if (m_lv->isLvmMirror()){

            QList<LogVol *> legs = m_lv->getChildren();
            int next_stripes = 0;

            for (int x = legs.size() - 1; x >= 0; x--){
                if (legs[x]->isMirrorLeg()) {

                    next_stripes = legs[x]->getSegmentStripes(legs[x]->getSegmentCount() - 1);
                    
                    if (next_stripes < stripes)
                        stripes = next_stripes;
                }
            }
        }
    }

    if (stripes < 1)
        stripes = 1;

    return stripes;
}

// This function tries to extend the pvs of the last segment, if they have been selected,
// before applying the usual methods of adding segments. lvextend with "normal" allocation
// seems to work this way.

void LVCreateDialog::extendLastSegment(QList<long long> &committed, QList<long long> &available)
{
    QList<LogVol *> legs;
    QStringList selected_names(m_pv_box->getNames());
    LogVol *lv = m_lv;

    if (lv->isThinPool()) {
        legs = lv->getChildren();
        for (int x = legs.size() - 1; x >= 0; x--) {
            if (legs[x]->isThinPoolData()) {
                lv = legs[x];
                break; 
            }
        }
    }

    legs.clear();

    if (lv->isMirror()) {
        legs = lv->getChildren();    // not grandchildren because we can't extend while under conversion
        
        for (int x = legs.size() - 1; x >= 0; x--) {
            if (!legs[x]->isMirrorLeg())
                legs.removeAt(x);
        }
    } else {
        legs.append(lv);
    }

    int commit_count = 0;

    for (int x = legs.size() - 1; x >= 0; x--) {

        const QStringList pv_names = legs[x]->getPvNames(legs[x]->getSegmentCount() - 1);

        for (int y = selected_names.size() - 1; y >= 0; y--) {
            for (int z = pv_names.size() - 1; z >= 0; z--) {

                if ((selected_names[y] == pv_names[z]) && commit_count < committed.size()) {
                    committed[commit_count] += available.takeAt(y);
                    selected_names.removeAt(y);
                    commit_count++;
                    break;
                }
            }
        }
    }
}

