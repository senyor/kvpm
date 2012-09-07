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
#include <KTabWidget>

#include <QCheckBox>
#include <QDebug>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>



/* This class handles both the creation and extension of logical
   volumes and snapshots since both processes are so similar. */

LVCreateDialog::LVCreateDialog(VolGroup *const group, QWidget *parent):
    KDialog(parent),
    m_vg(group)
{
    m_lv = NULL;
    m_extend = false;
    m_snapshot = false;
    m_bailout  = hasInitialErrors();
    m_fs_can_extend = false;

    if (!m_bailout)
        buildDialog();
}

LVCreateDialog::LVCreateDialog(LogVol *const volume, const bool snapshot, QWidget *parent):
    KDialog(parent),
    m_snapshot(snapshot),
    m_lv(volume)
{
    m_extend = !m_snapshot;
    m_vg = m_lv->getVg();
    m_bailout = hasInitialErrors();

    if (!m_bailout)
        buildDialog();
}

void LVCreateDialog::buildDialog()
{
    KConfigSkeleton skeleton;
    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", m_use_si_units, false);

    QLabel *const lv_name_label = new QLabel();
    lv_name_label->setAlignment(Qt::AlignCenter);
    
    if (m_extend) {
        setCaption(i18n("Extend Logical Volume"));
        lv_name_label->setText(i18n("<b>Extend volume: %1</b>", m_lv->getName()));
    } else if (m_snapshot) {
        setCaption(i18n("Create Snapshot Volume"));
        lv_name_label->setText(i18n("<b>Create snapshot of: %1</b>", m_lv->getName()));
    } else {
        setCaption(i18n("Create A New Logical Volume"));
        lv_name_label->setText(i18n("<b>Create a new logical volume</b>"));
    }
    
    QWidget *const main_widget = new QWidget();
    QVBoxLayout *const layout = new QVBoxLayout();
    
    m_tab_widget = new KTabWidget(this);
    m_physical_tab = createPhysicalTab();  // this order is important
    m_advanced_tab = createAdvancedTab();
    m_general_tab  = createGeneralTab();
    m_tab_widget->addTab(m_general_tab,  i18nc("The standard common options", "General"));
    m_tab_widget->addTab(m_physical_tab, i18n("Physical layout"));
    m_tab_widget->addTab(m_advanced_tab, i18nc("Less used, dangerous or complex options", "Advanced options"));
    
    layout->addWidget(lv_name_label);
    layout->addSpacing(5);
    layout->addWidget(m_tab_widget);
    main_widget->setLayout(layout);
    
    setMaxSize();

    enableTypeOptions(m_type_combo->currentIndex());
    enableStripeCombo(m_stripe_count_spin->value());
    makeConnections();
    resetOkButton();

    setMainWidget(main_widget);
}

void LVCreateDialog::makeConnections()
{
    connect(this, SIGNAL(okClicked()),
            this, SLOT(commitChanges()));

    connect(m_persistent_box, SIGNAL(toggled(bool)),
            this, SLOT(resetOkButton()));

    connect(m_major_edit, SIGNAL(textEdited(QString)),
            this, SLOT(resetOkButton()));

    connect(m_minor_edit, SIGNAL(textEdited(QString)),
            this, SLOT(resetOkButton()));

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

    connect(m_size_selector, SIGNAL(stateChanged()),
            this, SLOT(setMaxSize()));
}

QWidget* LVCreateDialog::createGeneralTab()
{
    m_tag_edit = NULL;

    QWidget *const general_tab = new QWidget(this);
    QHBoxLayout *const general_layout = new QHBoxLayout;
    general_tab->setLayout(general_layout);

    QVBoxLayout *const layout = new QVBoxLayout();
    general_layout->addStretch();
    general_layout->addLayout(layout);
    general_layout->addStretch();

    QGroupBox *const volume_box = new QGroupBox();
    QVBoxLayout *const volume_layout = new QVBoxLayout;
    volume_box->setLayout(volume_layout);

    layout->addWidget(volume_box);

    KLocale::BinaryUnitDialect dialect;
    KLocale *const locale = KGlobal::locale();

    if (m_use_si_units)
        dialect = KLocale::MetricBinaryDialect;
    else
        dialect = KLocale::IECBinaryDialect;

    if (m_extend) {
        m_extend_by_label = new QLabel();
        volume_layout->insertWidget(1, m_extend_by_label);
        m_current_size_label = new QLabel(i18n("Current size: %1", locale->formatByteSize(m_lv->getSize(), 1, dialect)));
        volume_layout->insertWidget(2, m_current_size_label);
    } else {
        QHBoxLayout *const name_layout = new QHBoxLayout();
        m_name_edit = new KLineEdit();

        QRegExp rx("[0-9a-zA-Z_\\.][-0-9a-zA-Z_\\.]*");
        m_name_validator = new QRegExpValidator(rx, m_name_edit);
        m_name_edit->setValidator(m_name_validator);
        QLabel *const name_label = new QLabel(i18n("Volume name: "));
        name_label->setBuddy(m_name_edit);
        name_layout->addWidget(name_label);
        name_layout->addWidget(m_name_edit);
        volume_layout->insertLayout(0, name_layout);

        QHBoxLayout *const tag_layout = new QHBoxLayout();
        m_tag_edit = new KLineEdit();

        QRegExp rx2("[0-9a-zA-Z_\\.+-]*");
        m_tag_validator = new QRegExpValidator(rx2, m_tag_edit);
        m_tag_edit->setValidator(m_tag_validator);
        QLabel *const tag_label = new QLabel(i18n("Optional tag: "));
        tag_label->setBuddy(m_tag_edit);
        tag_layout->addWidget(tag_label);
        tag_layout->addWidget(m_tag_edit);
        volume_layout->insertLayout(1, tag_layout);

        connect(m_name_edit, SIGNAL(textEdited(QString)),
                this, SLOT(resetOkButton()));

    }

    if (m_extend) {
        m_size_selector = new SizeSelectorBox(m_vg->getExtentSize(), m_lv->getExtents(),
                                              m_vg->getAllocatableExtents() + m_lv->getExtents(),
                                              m_lv->getExtents(), true, false);
    } else {
        m_size_selector = new SizeSelectorBox(m_vg->getExtentSize(), 0,
                                              m_vg->getAllocatableExtents(),
                                              0, true, false);
    }

    layout->addWidget(m_size_selector);

    QGroupBox *const misc_box = new QGroupBox();
    QVBoxLayout *const misc_layout = new QVBoxLayout();
    misc_box->setLayout(misc_layout);

    m_readonly_check = new QCheckBox();
    m_readonly_check->setText(i18n("Set read only"));
    misc_layout->addWidget(m_readonly_check);
    m_zero_check = new QCheckBox();
    m_zero_check->setText(i18n("Write zeros at volume start"));
    misc_layout->addWidget(m_zero_check);
    misc_layout->addStretch();

    if (!m_snapshot && !m_extend) {

        connect(m_zero_check, SIGNAL(stateChanged(int)),
                this , SLOT(zeroReadonlyCheck(int)));
        connect(m_readonly_check, SIGNAL(stateChanged(int)),
                this , SLOT(zeroReadonlyCheck(int)));

        m_zero_check->setChecked(true);
        m_readonly_check->setChecked(false);
    } else if (m_snapshot && !m_extend) {
        m_zero_check->setChecked(false);
        m_zero_check->setEnabled(false);
        m_zero_check->hide();
        m_readonly_check->setChecked(false);
    } else {
        m_zero_check->setChecked(false);
        m_zero_check->setEnabled(false);
        m_readonly_check->setChecked(false);
        m_readonly_check->setEnabled(false);
        m_zero_check->hide();
        m_readonly_check->hide();
    }

    m_max_size_label = new QLabel();
    misc_layout->addWidget(m_max_size_label);
    m_max_extents_label = new QLabel();
    misc_layout->addWidget(m_max_extents_label);
    m_stripe_count_label = new QLabel();
    m_stripe_count_label->setWordWrap(true);
    misc_layout->addWidget(m_stripe_count_label);
    layout->addWidget(misc_box);

    return general_tab;
}

QWidget* LVCreateDialog::createPhysicalTab()
{
    QString message;


    QVBoxLayout *const layout = new QVBoxLayout;
    m_physical_tab = new QWidget(this);
    m_physical_tab->setLayout(layout);

    QList<PhysVol *> physical_volumes(m_vg->getPhysicalVolumes());

    for (int x = physical_volumes.size() - 1; x >= 0; x--) {
        if (physical_volumes[x]->getRemaining() < 1 || !physical_volumes[x]->isAllocatable())
            physical_volumes.removeAt(x);
    }

    QList<long long> normal;
    QList<long long> contiguous;

    if (m_lv != NULL) {

        for (int x = 0; x < physical_volumes.size(); x++) {
            normal.append(physical_volumes[x]->getRemaining());
            contiguous.append(physical_volumes[x]->getContiguous(m_lv));
        }

        m_pv_box = new PvGroupBox(physical_volumes, normal, contiguous, m_lv->getPolicy());
    } else {

        for (int x = 0; x < physical_volumes.size(); x++) {
            normal.append(physical_volumes[x]->getRemaining());
            contiguous.append(physical_volumes[x]->getContiguous(m_lv));
        }

        m_pv_box = new PvGroupBox(physical_volumes, normal, contiguous, INHERITED);
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

    volume_layout->addWidget(createTypeWidget(physical_volumes.size()));
    m_stripe_widget = createStripeWidget();
    m_mirror_widget = createMirrorWidget(physical_volumes.size());
    volume_layout->addWidget(m_stripe_widget);
    volume_layout->addWidget(m_mirror_widget);
    volume_layout->addStretch();

    lower_v_layout->addWidget(m_volume_box);

    return m_physical_tab;
}

QWidget* LVCreateDialog::createAdvancedTab()
{
    QHBoxLayout *const advanced_layout = new QHBoxLayout;
    m_advanced_tab = new QWidget(this);
    m_advanced_tab->setLayout(advanced_layout);
    QGroupBox *const advanced_box = new QGroupBox();
    QVBoxLayout *const layout = new QVBoxLayout;
    advanced_box->setLayout(layout);
    advanced_layout->addStretch();
    advanced_layout->addWidget(advanced_box);
    advanced_layout->addStretch();

    m_monitor_check = new QCheckBox(i18n("Monitor with dmeventd"));
    m_skip_sync_check = new QCheckBox(i18n("Skip initial synchronization of mirror"));
    m_skip_sync_check->setChecked(false);
    layout->addWidget(m_monitor_check);
    layout->addWidget(m_skip_sync_check);

    if (m_snapshot) {
        m_monitor_check->setChecked(true);
        m_monitor_check->setEnabled(true);
        m_skip_sync_check->setEnabled(false);
    } else if (m_extend) {
        m_monitor_check->setChecked(false);
        m_monitor_check->setEnabled(false);
        m_skip_sync_check->setEnabled(false);
        m_monitor_check->hide();
        m_skip_sync_check->hide();
    } else {
        m_monitor_check->setChecked(false);
        m_monitor_check->setEnabled(false);
        m_skip_sync_check->setEnabled(false);
    }

    m_udevsync_check = new QCheckBox(i18n("Synchronize with udev"));
    m_udevsync_check->setChecked(true);
    layout->addWidget(m_udevsync_check);

    QVBoxLayout *const persistent_layout   = new QVBoxLayout;
    QHBoxLayout *const minor_number_layout = new QHBoxLayout;
    QHBoxLayout *const major_number_layout = new QHBoxLayout;
    m_minor_edit = new KLineEdit();
    m_major_edit = new KLineEdit();
    QLabel *const minor_number = new QLabel(i18n("Device minor number: "));
    QLabel *const major_number = new QLabel(i18n("Device major number: "));
    minor_number->setBuddy(m_minor_edit);
    major_number->setBuddy(m_major_edit);
    major_number_layout->addWidget(major_number);
    major_number_layout->addWidget(m_major_edit);
    minor_number_layout->addWidget(minor_number);
    minor_number_layout->addWidget(m_minor_edit);
    persistent_layout->addLayout(major_number_layout);
    persistent_layout->addLayout(minor_number_layout);
    QIntValidator *const minor_validator = new QIntValidator(m_minor_edit);
    QIntValidator *const major_validator = new QIntValidator(m_major_edit);
    minor_validator->setBottom(0);
    major_validator->setBottom(0);
    m_minor_edit->setValidator(minor_validator);
    m_major_edit->setValidator(major_validator);

    m_persistent_box = new QGroupBox(i18n("Use persistent device numbering"));
    m_persistent_box->setCheckable(true);
    m_persistent_box->setChecked(false);
    m_persistent_box->setLayout(persistent_layout);
    layout->addWidget(m_persistent_box);

    if (m_extend)
        m_persistent_box->hide();

    layout->addStretch();

    return m_advanced_tab;
}

QWidget* LVCreateDialog::createStripeWidget()
{
    QWidget *const widget = new QWidget;
    QVBoxLayout *const layout = new QVBoxLayout;

    m_stripe_size_combo = new KComboBox();
    m_stripe_size_combo->setEnabled(false);
    for (int n = 2; (pow(2, n) * 1024) <= m_vg->getExtentSize() ; n++) {
        m_stripe_size_combo->addItem(QString("%1").arg(pow(2, n)) + " KiB");
        m_stripe_size_combo->setItemData(n - 2, QVariant((int) pow(2, n)), Qt::UserRole);
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
                    m_stripe_count_spin->setSpecialValueText(i18n(""));
                } else if (m_lv->getRaidType() == 6) {
                    stripe_count -= 2;
                    m_stripe_count_spin->setSuffix(i18n(" + 2 parity"));
                    m_stripe_count_spin->setSpecialValueText(i18n(""));
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

    if(m_extend) {
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
    } else {
        
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
    KLocale::BinaryUnitDialect dialect;
    KLocale *const locale = KGlobal::locale();

    if (m_use_si_units)
        dialect = KLocale::MetricBinaryDialect;
    else
        dialect = KLocale::IECBinaryDialect;

    m_stripe_count_spin->setMaximum(getMaxStripes());

    const int stripe_count = m_stripe_count_spin->value();
    const int mirror_count = m_mirror_count_spin->value();
    const long long max_extents = getLargestVolume() / m_vg->getExtentSize();

    m_size_selector->setConstrainedMax(max_extents);
    m_max_size_label->setText(i18n("Maximum volume size: %1", locale->formatByteSize(getLargestVolume(), 1, dialect)));
    m_max_extents_label->setText(i18n("Maximum volume extents: %1", max_extents));

    if (m_type_combo->currentIndex() == 0){
        if (m_stripe_count_spin->value() > 1)
            m_stripe_count_label->setText(i18n("(with %1 stripes)", stripe_count));
        else
            m_stripe_count_label->setText(i18n("(linear volume)"));
    }
    else if (m_type_combo->currentIndex() == 1){
        if (m_stripe_count_spin->value() > 1)
            m_stripe_count_label->setText(i18n("(LVM2 mirror with %1 legs and %2 stripes)", mirror_count, stripe_count));
        else
            m_stripe_count_label->setText(i18n("(LVM2 mirror with %1 legs)", mirror_count));
    }
    else if (m_type_combo->currentIndex() == 2){
        m_stripe_count_label->setText(i18n("(RAID 1 mirror with %1 legs)", mirror_count));
    }
    else if (m_type_combo->currentIndex() == 3){
        m_stripe_count_label->setText(i18n("(RAID 4 with %1 stripes + 1 parity)", stripe_count));
    }
    else if (m_type_combo->currentIndex() == 4){
        m_stripe_count_label->setText(i18n("(RAID 5 with %1 stripes + 1 parity)", stripe_count));
    }
    else if (m_type_combo->currentIndex() == 5){
        m_stripe_count_label->setText(i18n("(RAID 6 with %1 stripes + 2 parity)", stripe_count));
    }
    
    resetOkButton();
}

void LVCreateDialog::resetOkButton()
{
    bool valid_name  = true;
    bool valid_major = true;
    bool valid_minor = true;

    if (!m_extend) {
        int pos = 0;
        QString name = m_name_edit->text();
        
        if (m_name_validator->validate(name, pos) == QValidator::Acceptable && name != "." && name != "..")
            valid_name = true;
        else if (name.isEmpty())
            valid_name = true;
        else
            valid_name = false;
        
        QString major = m_major_edit->text();
        QString minor = m_minor_edit->text();
        const QValidator *const major_validator = m_major_edit->validator();
        const QValidator *const minor_validator = m_minor_edit->validator();
        valid_major = (major_validator->validate(major, pos) == QValidator::Acceptable); 
        valid_minor = (minor_validator->validate(minor, pos) == QValidator::Acceptable); 
    }

    const long long max = getLargestVolume() / m_vg->getExtentSize();
    const long long selected = m_size_selector->getCurrentSize();
    const long long rounded  = roundExtentsToStripes(selected);

    if (m_persistent_box->isChecked() && !(valid_major && valid_minor)) {
        enableButtonOk(false);
    } else if (m_size_selector->isValid() && valid_name) {
        if (!m_extend) {
            if ((rounded <= max) && (rounded > 0))
                enableButtonOk(true);
            else
                enableButtonOk(false);
        } else {
            if ((rounded <= max) && (rounded > m_lv->getExtents()) && (selected > m_lv->getExtents()))
                enableButtonOk(true);
            else
                enableButtonOk(false);
        }
    } else
        enableButtonOk(false);
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
        m_stripe_count_spin->setValue(1);
        m_stripe_count_spin->setSuffix(i18n(""));
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
            m_stripe_count_spin->setSuffix(i18n(""));
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
        m_stripe_count_spin->setSuffix(i18n(""));
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
    if (index == 1 || index == 2) {           // lvm mirror or raid mirror
        m_monitor_check->setChecked(true);    // whether a snap or not
        m_monitor_check->setEnabled(true);
        m_skip_sync_check->setEnabled(true);
    } else if (index > 2 || m_snapshot){      // raid stripe set
        m_monitor_check->setChecked(true);    // and all other snaps
        m_monitor_check->setEnabled(true);
        m_skip_sync_check->setChecked(false);
        m_skip_sync_check->setEnabled(false);
    } else {                                  // linear
        m_monitor_check->setChecked(false);
        m_monitor_check->setEnabled(false);
        m_skip_sync_check->setChecked(false);
        m_skip_sync_check->setEnabled(false);
    }
}

/* largest volume that can be created given the pvs , striping and mirrors
   selected. This includes the size of the already existing volume if we
   are extending a volume */

long long LVCreateDialog::getLargestVolume()
{
    QList <long long> available_pv_bytes = m_pv_box->getRemainingSpaceList();
    QList <long long> stripe_pv_bytes;
    int total_stripes;
    const int type = m_type_combo->currentIndex();
    const int stripe_count = m_stripe_count_spin->value();
    const int mirror_count = m_mirror_count_spin->value();
    const long long extent_size = m_vg->getExtentSize();

    if (type == 1)           // LVM2 mirror
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

    for (int x = 0; x < total_stripes; x++)
        stripe_pv_bytes.append(0);

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

    qSort(available_pv_bytes);

    AllocationPolicy policy = m_pv_box->getPolicy();
    if (policy == INHERITED)
        policy = m_vg->getPolicy();

    if (!m_extend) {
        if (policy == CONTIGUOUS) {
            while (available_pv_bytes.size() > total_stripes + log_count)  
                available_pv_bytes.removeFirst();
        } 
    }

    for (int x = 0; x < log_count; x++) {
        if (available_pv_bytes.size())
            available_pv_bytes.removeFirst();
        else
            return 0;
    }

    while (available_pv_bytes.size()) {
        qSort(stripe_pv_bytes);
        stripe_pv_bytes[0] += available_pv_bytes.takeLast();
    }
    qSort(stripe_pv_bytes);

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
                        stripe_max.append(m_vg->getPvByName(pv_names[y])->getContiguous(m_lv));

                    qSort(stripe_max);

                    if (stripe_max.size() < stripe_count)
                        return m_lv->getSize();

                    while (stripe_max.size() > stripe_count)
                        stripe_max.removeFirst();

                    leg_max.append(stripe_max[0] * stripe_count);
                }
            }

            qSort(leg_max);
            largest = leg_max[0] +m_lv->getSize();

        } else {
            largest = (largest * stripe_count) + m_lv->getSize();
        }
    }

    if (largest < 0)
        largest = 0;

    return largest;
}

void LVCreateDialog::zeroReadonlyCheck(int)
{
    if (!m_snapshot && !m_extend) {
        if (m_zero_check->isChecked()) {
            m_readonly_check->setChecked(false);
            m_readonly_check->setEnabled(false);
        } else
            m_readonly_check->setEnabled(true);

        if (m_readonly_check->isChecked()) {
            m_zero_check->setChecked(false);
            m_zero_check->setEnabled(false);
        } else
            m_zero_check->setEnabled(true);
    } else
        m_readonly_check->setEnabled(true);
}

/* Here we create a stringlist of arguments based on all
   the options that the user chose in the dialog. */

QStringList LVCreateDialog::argumentsLV()
{
    QString program_to_run;
    QStringList args;
    const QVariant stripe_size = m_stripe_size_combo->itemData(m_stripe_size_combo->currentIndex(), Qt::UserRole);
    const int stripes = m_stripe_count_spin->value();
    const int type    = m_type_combo->currentIndex();
    const int mirrors = m_mirror_count_spin->value();
    long long extents = m_size_selector->getCurrentSize();

    if (m_tag_edit) {
        if (!(m_tag_edit->text()).isEmpty())
            args << "--addtag" << m_tag_edit->text();
    }

    if (!m_udevsync_check->isChecked())
        args << "--noudevsync";

    if (m_persistent_box->isChecked()) {
        args << "--persistent" << "y";
        args << "--major" << m_major_edit->text();
        args << "--minor" << m_minor_edit->text();
    }

    if (stripes > 1) {
        args << "--stripes" << QString("%1").arg(stripes);
        args << "--stripesize" << QString("%1").arg(stripe_size.toLongLong());
    }

    if (!m_extend) {
        if (m_readonly_check->isChecked())
            args << "--permission" << "r" ;
        else
            args << "--permission" << "rw" ;

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
            if (m_zero_check->isChecked())
                args << "--zero" << "y";
            else
                args << "--zero" << "n";

            if (mirrors > 1) {
                args << "--mirrors" << QString("%1").arg(mirrors - 1);

                if (m_skip_sync_check->isChecked())
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

    if (m_monitor_check->isEnabled()) {
        args << "--monitor";
        if (m_monitor_check->isChecked())
            args << "y";
        else
            args << "n";
    }

    const QString policy = policyToString(m_pv_box->getPolicy());

    if (policy != "inherited")          // "inherited" is what we get if we don't pass "--alloc" at all
        args << "--alloc" << policy;    // passing "--alloc" "inherited" won't work

    if (m_extend)
        extents -= m_lv->getExtents();

    extents = roundExtentsToStripes(extents);

    args << "--extents" << QString("+%1").arg(extents);

    if (!m_extend && !m_snapshot) {                           // create a standard volume
        program_to_run = "lvcreate";

        if (!(m_name_edit->text()).isEmpty())
            args << "--name" << m_name_edit->text();

        args << m_vg->getName();
    } else if (m_snapshot) {                                // create a snapshot
        program_to_run = "lvcreate";

        args << "--snapshot";

        if (!(m_name_edit->text()).isEmpty())
            args << "--name" << m_name_edit->text() ;
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
    const long long max_extents = getLargestVolume() / m_vg->getExtentSize();

    // The next part should only need to reference stripes, not the mirror count
    // but a bug in lvm requires it. Remove this when fixed.

    if (stripes > 1) {
        if (extents % (stripes * mirrors)) {
            extents = extents / (stripes * mirrors);
            extents = extents * (stripes * mirrors);
            if (extents + (stripes * mirrors) <= max_extents)
                extents += (stripes * mirrors);
        }
    }

    return extents;
}

// This function checks for problems that would make showing this dialog pointless
// returns true if there are problems and is used to set m_bailout.

bool LVCreateDialog::hasInitialErrors()
{
    if (!m_vg->getAllocatableExtents()) {
        if (m_vg->getFreeExtents())
            KMessageBox::error(this, i18n("All free physical volume extents in this group"
                                          " are locked against allocation"));
        else
            KMessageBox::error(this, i18n("There are no allocatable free extents in this volume group"));

        return true;
    }

    if (m_extend) {

        const QString warning_message = i18n("If this volume has a filesystem or data, it will need to be extended <b>separately</b>. "
                                             "Currently, only the ext2, ext3, ext4, xfs, jfs, ntfs and reiserfs file systems are "
                                             "supported for extension. The correct executable for extension must also be present. ");

        if (m_lv->isRaid() || m_lv->isLvmMirror()){
            QList<PhysVol *> pvs = m_vg->getPhysicalVolumes();

            for (int x = pvs.size() - 1; x >= 0; x--) {
                if (pvs[x]->getRemaining() < 1 || !pvs[x]->isAllocatable())
                    pvs.removeAt(x);
            }

            if (m_lv->getMirrorCount() > pvs.size() ||  (m_lv->isRaid() && m_lv->getSegmentStripes(0) > pvs.size())) {
                KMessageBox::error(this, i18n("Insufficient allocatable physical volumes to extend this volume"));
                return true;
            }
        }

        if (m_lv->isOrigin()) {
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

        m_fs_can_extend = fs_can_extend(m_lv->getFilesystem());

        if (!(m_fs_can_extend || m_lv->isSnap())) {
            if (KMessageBox::warningContinueCancel(NULL,
                                                   warning_message,
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

void LVCreateDialog::commitChanges()
{
    QStringList lvchange_args;
    hide();

    if (!m_extend) {
        ProcessProgress create_lv(argumentsLV());
        return;
    } else {
        const QString mapper_path = m_lv->getMapperPath();
        const QString fs = m_lv->getFilesystem();

        if (m_lv->isOrigin()) {

            lvchange_args << "lvchange" << "-an" << mapper_path;
            ProcessProgress deactivate_lv(lvchange_args);
            if (deactivate_lv.exitCode()) {
                KMessageBox::error(0, i18n("Volume deactivation failed, volume not extended"));
                return;
            }

            ProcessProgress extend_origin(argumentsLV());
            if (extend_origin.exitCode()) {
                KMessageBox::error(0, i18n("Volume extension failed"));
                return;
            }

            lvchange_args.clear();
            lvchange_args << "lvchange" << "-ay" << mapper_path;
            ProcessProgress activate_lv(lvchange_args);
            if (activate_lv.exitCode()) {
                KMessageBox::error(0, i18n("Volume activation failed, filesystem not extended"));
                return;
            }

            if (m_fs_can_extend)
                fs_extend(m_lv->getMapperPath(), fs, m_lv->getMountPoints(), true);

            return;
        } else {
            ProcessProgress extend_lv(argumentsLV());
            if (!extend_lv.exitCode() && !m_lv->isSnap() && m_fs_can_extend)
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
        policy = m_vg->getPolicy();

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

    return stripes;
}


    
