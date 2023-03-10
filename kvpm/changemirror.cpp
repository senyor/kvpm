/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "changemirror.h"

#include "logvol.h"
#include "lvmconfig.h"
#include "misc.h"
#include "physvol.h"
#include "pvgroupbox.h"
#include "processprogress.h"
#include "volgroup.h"

#include <cmath>

#include <KLocalizedString>
#include <KMessageBox>

#include <QApplication>
#include <QComboBox>
#include <QEventLoop>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTabWidget>
#include <QVBoxLayout>



ChangeMirrorDialog::ChangeMirrorDialog(LogVol *const mirrorVolume, const bool changeLog, QWidget *parent) :
    KvpmDialog(parent),
    m_change_log(changeLog),
    m_lv(mirrorVolume)
{
    if (m_lv->getParentMirror())
        m_lv = m_lv->getParentMirror();
    else if (m_lv->getParentRaid())
        m_lv = m_lv->getParentRaid();

    QList<LogVol *> children;
    const QString lv_name = m_lv->getName();
    const bool is_raid = (m_lv->isMirror() && m_lv->isRaid());
    const bool is_lvm = (m_lv->isMirror() && !m_lv->isRaid());

    m_log_pvs = getLogPvs();
    m_image_pvs = getImagePvs();

    setCaption(i18n("Change Mirror"));

    if ((is_raid && !m_lv->isMirror()) || m_lv->isCowSnap()) {
        preventExec();
        KMessageBox::sorry(nullptr, i18n("This type of volume can not be mirrored "));
    } else if (is_raid && !m_lv->isSynced()) {
        preventExec();
        KMessageBox::sorry(nullptr, i18n("RAID mirrors must be synced before adding new legs"));
    } else if (is_lvm && m_lv->isCowOrigin() && !m_change_log) {
        preventExec();
        KMessageBox::sorry(nullptr, i18n("Non-RAID mirrors which are snapshot origins can not have new legs added"));
    } else {
        QWidget *const main_widget = new QWidget();
        QVBoxLayout *const layout = new QVBoxLayout();
        QLabel  *const lv_name_label = new QLabel();
        
        if(m_change_log)
            lv_name_label->setText(i18n("Change mirror log: %1", m_lv->getName()));
        else if(is_lvm)
            lv_name_label->setText(i18n("Change LVM mirror: %1", m_lv->getName()));
        else if(is_raid)
            lv_name_label->setText(i18n("Change RAID 1 mirror: %1", m_lv->getName()));
        else
            lv_name_label->setText(i18n("Convert to a mirror: %1", m_lv->getName()));
        
        lv_name_label->setAlignment(Qt::AlignCenter);
        m_tab_widget = new QTabWidget();
        layout->addWidget(lv_name_label);
        layout->addSpacing(10);
        layout->addWidget(m_tab_widget);
        main_widget->setLayout(layout);
        
        setMainWidget(main_widget);
        
        m_tab_widget->addTab(buildGeneralTab(is_raid, is_lvm), i18nc("Common user options", "General"));
        
        if (!(m_change_log && (m_lv->getLogCount() == 2))) {
            
            m_tab_widget->addTab(buildPhysicalTab(is_raid), i18n("Physical layout"));
            
            connect(m_pv_box, SIGNAL(stateChanged()),
                    this, SLOT(resetOkButton()));
            
            connect(m_add_mirrors_spin, SIGNAL(valueChanged(int)),
                    this, SLOT(resetOkButton()));
            
            connect(m_stripe_spin, SIGNAL(valueChanged(int)),
                    this, SLOT(resetOkButton()));
            
            connect(m_stripe_box, SIGNAL(toggled(bool)),
                    this, SLOT(resetOkButton()));
        } else {
            m_pv_box = nullptr;
        }
        
        setLogRadioButtons();
        
        connect(m_disk_log_button, SIGNAL(toggled(bool)),
                this, SLOT(resetOkButton()));
        
        connect(m_core_log_button, SIGNAL(toggled(bool)),
                this, SLOT(resetOkButton()));
        
        connect(m_mirrored_log_button, SIGNAL(toggled(bool)),
                this, SLOT(resetOkButton()));

        enableTypeOptions(m_type_combo->currentIndex());

        if (!m_change_log && (m_space_list.size() < 1)) {
            preventExec();
            KMessageBox::sorry(nullptr, i18n("No physical volumes suitable for a new mirror image found"));
        }
    }
}

QWidget *ChangeMirrorDialog::buildGeneralTab(const bool isRaidMirror, const bool isLvmMirror)
{
    QWidget *const general = new QWidget;
    QHBoxLayout *const general_layout = new QHBoxLayout;
    QVBoxLayout *const center_layout = new QVBoxLayout;
    const bool is_mirror = (isRaidMirror || isLvmMirror);

    m_type_combo = new QComboBox();
    m_type_combo->addItem(i18n("Standard LVM"));
    m_type_combo->addItem(i18n("RAID 1"));
    m_add_mirrors_spin = new QSpinBox(this);
    m_add_mirrors_spin->setMaximum(10);
    m_add_mirrors_spin->setMinimum(1);
    m_add_mirrors_spin->setSingleStep(1);
    m_add_mirrors_spin->setValue(1);

    general_layout->addStretch();
    general_layout->addLayout(center_layout);
    general_layout->addStretch();
        
    QGroupBox *const add_mirror_box = new QGroupBox();
    QLabel *const type_label = new QLabel(i18n("Mirror type: "));
    
    QHBoxLayout *const type_layout = new QHBoxLayout;
    type_layout->addWidget(type_label);
    type_layout->addWidget(m_type_combo);
    
    QVBoxLayout *const add_mirror_box_layout = new QVBoxLayout;
    add_mirror_box_layout->addLayout(type_layout);
    add_mirror_box->setLayout(add_mirror_box_layout);
    center_layout->addStretch();
    center_layout->addWidget(add_mirror_box);

    QLabel *const existing_label = new QLabel(i18n("Existing mirror legs: %1", m_lv->getMirrorCount()));
    add_mirror_box_layout->addWidget(existing_label);
    if (!is_mirror)
        existing_label->hide();
    
    QHBoxLayout *const spin_box_layout = new QHBoxLayout();
    QLabel *const add_mirrors_label = new QLabel(i18n("Add mirror legs: "));
    
    add_mirrors_label->setBuddy(m_add_mirrors_spin);
    spin_box_layout->addWidget(add_mirrors_label);
    spin_box_layout->addWidget(m_add_mirrors_spin);
    add_mirror_box_layout->addLayout(spin_box_layout);
    add_mirror_box_layout->addStretch();
    
    if (m_change_log) {
        m_type_combo->setCurrentIndex(0);
        m_type_combo->hide();
        type_label->hide();
        add_mirror_box->hide();
    } else {
        if(is_mirror) {
            add_mirror_box->setTitle(i18n("Add mirror legs"));
            m_type_combo->hide();
            type_label->hide();
        } else {
            add_mirror_box->setTitle(i18n("Convert to mirror"));

            if (LvmConfig::getMirrorSegtypeDefault() == QString("mirror"))
                m_type_combo->setCurrentIndex(0);
            else
                m_type_combo->setCurrentIndex(1);

            connect(m_type_combo, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(enableTypeOptions(int)));

            connect(m_type_combo, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(resetOkButton()));
        }
    }

    m_log_box = new QGroupBox(i18n("Mirror logging"));
    QVBoxLayout *const log_box_layout = new QVBoxLayout;
    m_core_log_button = new QRadioButton(i18n("Memory based log"));
    m_disk_log_button = new QRadioButton(i18n("Single disk based log"));
    m_mirrored_log_button = new QRadioButton(i18n("Mirrored disk based log"));
    
    log_box_layout->addWidget(m_mirrored_log_button);
    log_box_layout->addWidget(m_disk_log_button);
    log_box_layout->addWidget(m_core_log_button);
    m_log_box->setLayout(log_box_layout);
    center_layout->addWidget(m_log_box);

    if (!is_mirror || m_change_log) {
        if (m_change_log) {
            m_log_box->setEnabled(true);

            if (m_lv->getLogCount() == 0)
                m_core_log_button->setChecked(true);
            else if (m_lv->getLogCount() == 1)
                m_disk_log_button->setChecked(true);
            else
                m_mirrored_log_button->setChecked(true);
        } else {
            m_disk_log_button->setChecked(true);
        }
    } else {
        m_log_box->hide();
    }

    if (m_change_log && (m_lv->getLogCount() == 2)) {
        m_log_widget = buildLogWidget();
        center_layout->addWidget(m_log_widget);
        enableLogWidget();

        connect(m_disk_log_button, SIGNAL(toggled(bool)),
                this, SLOT(enableLogWidget()));
        
        connect(m_core_log_button, SIGNAL(toggled(bool)),
                this, SLOT(enableLogWidget()));
        
        connect(m_mirrored_log_button, SIGNAL(toggled(bool)),
                this, SLOT(enableLogWidget()));
    } else {
        m_log_widget = nullptr;
    }

    center_layout->addStretch();
    general->setLayout(general_layout);

    return general;
}

QWidget *ChangeMirrorDialog::buildLogWidget()
{
    QWidget *const no_change = new QWidget();
    QVBoxLayout *const no_change_layout = new QVBoxLayout();
    QLabel *const no_change_label = new QLabel(i18n("No change"));
    no_change_label->setAlignment(Qt::AlignCenter);
    no_change_layout->addWidget(no_change_label);
    no_change->setLayout(no_change_layout);

    QWidget *const no_log = new QWidget();
    QVBoxLayout *const no_log_layout = new QVBoxLayout();
    QLabel *const no_log_label = new QLabel(i18n("Remove both disk logs"));
    no_log_label->setAlignment(Qt::AlignCenter);
    no_log_layout->addWidget(no_log_label);
    no_log->setLayout(no_log_layout);

    QStringList names;
    for (auto log : m_lv->getAllChildrenFlat()) {
        if (log->isLvmMirrorLog() && !log->isMirror())
            names << log->getPvNamesAll();
    }

    if (names.size() > 0)
        m_log_one = new NoMungeRadioButton(names[0]);
    else
        m_log_one = new NoMungeRadioButton("");

    if (names.size() > 1)
        m_log_two = new NoMungeRadioButton(names[1]);
    else
        m_log_two = new NoMungeRadioButton("");

    m_log_one->setChecked(true);

    QWidget *const one_log = new QWidget();
    QVBoxLayout *const one_log_layout = new QVBoxLayout();
    one_log_layout->addWidget(m_log_one);
    one_log_layout->addWidget(m_log_two);
    one_log->setLayout(one_log_layout);

    m_log_stack = new QStackedWidget();
    m_log_stack->addWidget(no_change);
    m_log_stack->addWidget(one_log);
    m_log_stack->addWidget(no_log);

    QVBoxLayout *const layout = new QVBoxLayout();
    QGroupBox *const log_box = new QGroupBox("Mirror log to remove");
    layout->addWidget(m_log_stack);
    log_box->setLayout(layout);

    return log_box;
}

QWidget *ChangeMirrorDialog::buildPhysicalTab(const bool isRaidMirror)
{
    QWidget *const physical = new QWidget;
    QVBoxLayout *const physical_layout = new QVBoxLayout();

    m_space_list = getPvSpaceList();
    m_pv_box = new PvGroupBox(m_space_list, m_lv->getPolicy(), m_lv->getVg()->getPolicy());
    physical_layout->addWidget(m_pv_box);
    physical_layout->addStretch();

    QHBoxLayout *const h_layout = new QHBoxLayout();
    QVBoxLayout *const lower_layout = new QVBoxLayout();
    physical_layout->addLayout(h_layout);
    h_layout->addStretch();
    h_layout->addLayout(lower_layout);
    h_layout->addStretch();

    m_stripe_box = new QGroupBox(i18n("Volume striping"));
    QVBoxLayout *const striped_layout = new QVBoxLayout();
    m_stripe_box->setLayout(striped_layout);
    
    m_stripe_size_combo = new QComboBox();
    for (int n = 2; (pow(2, n) * 1024) <= m_lv->getVg()->getExtentSize() ; n++) {
        m_stripe_size_combo->addItem(QString("%1").arg(pow(2, n)) + " KiB");
        m_stripe_size_combo->setItemData(n - 2, QVariant((int) pow(2, n)), Qt::UserRole);

        if ((n - 2) < 5) 
            m_stripe_size_combo->setCurrentIndex(n - 2);
    }
    
    QLabel *const stripe_size = new QLabel(i18n("Stripe Size: "));
    m_stripe_spin = new QSpinBox();
    m_stripe_spin->setMinimum(1);
    m_stripe_spin->setSpecialValueText(i18n("none"));
    m_stripe_spin->setMaximum(m_lv->getVg()->getPvCount());
    stripe_size->setBuddy(m_stripe_size_combo);
    QHBoxLayout *const stripe_size_layout = new QHBoxLayout();
    stripe_size_layout->addWidget(stripe_size);
    stripe_size_layout->addWidget(m_stripe_size_combo);
    QLabel *const stripes_number = new QLabel(i18n("Number of stripes: "));
    stripes_number->setBuddy(m_stripe_spin);
    QHBoxLayout *const stripes_number_layout = new QHBoxLayout();
    stripes_number_layout->addWidget(stripes_number);
    stripes_number_layout->addWidget(m_stripe_spin);
    striped_layout->addLayout(stripe_size_layout);
    striped_layout->addLayout(stripes_number_layout);
    
    m_error_stack = new QStackedWidget();
    QWidget *const error_widget = new QWidget();
    QWidget *const blank_widget = new QWidget();
    m_error_stack->addWidget(error_widget);
    m_error_stack->addWidget(blank_widget);
    QHBoxLayout *const error_layout = new QHBoxLayout();
    QVBoxLayout *const error_right_layout = new QVBoxLayout();
    
    QLabel *const stripe_error1 = new QLabel("");
    stripe_error1->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-warning")).pixmap(32, 32));
    QLabel *const stripe_error2 = new QLabel(i18n("The number of extents: %1 must be evenly divisible by the number of stripes", m_lv->getExtents()));
    stripe_error2->setWordWrap(true);
    error_layout->addWidget(stripe_error1);
    error_right_layout->addWidget(stripe_error2);
    error_layout->addLayout(error_right_layout);
    error_widget->setLayout(error_layout);
    
    striped_layout->addWidget(m_error_stack);
    lower_layout->addWidget(m_stripe_box);
    
    if (m_change_log || isRaidMirror) {
        m_stripe_box->hide();
        m_stripe_box->setEnabled(false);
    }

    lower_layout->addStretch();
    physical->setLayout(physical_layout);

    return physical;
}

QStringList ChangeMirrorDialog::getLogPvs()
{
    QStringList pvs;

    if (m_lv->isLvmMirror()) {
        for (auto child : m_lv->getAllChildrenFlat()) {
            if (child->isLvmMirrorLog() && !child->isMirror())
                pvs << child->getPvNamesAll();
        } 
    } 
    pvs.removeDuplicates();

    return pvs;   
}
   
QStringList ChangeMirrorDialog::getImagePvs()
{
    QStringList pvs;

    if (m_lv->isMirror()) {
        for (auto child : m_lv->getAllChildrenFlat()) {
            if (child->isMirrorLeg() && !child->isMirror() && !child->isLvmMirrorLog())
                pvs << child->getPvNamesAll();
        } 
    } else {
        pvs << m_lv->getPvNamesAll();
    }
    pvs.removeDuplicates();

    return pvs;   
}

// If a pv holds all or part of a mirror image return true, else false.
// Also return true for pvs used by the current lv if that lv is being
// changed into a mirror.
bool ChangeMirrorDialog::pvHasImage(QString const pv)
{
    for (auto img_pv : m_image_pvs) {
        if (pv == img_pv)
            return true;
    }

    return false;
}

// If a pv holds all or part of a log return true, else false.
bool  ChangeMirrorDialog::pvHasLog(QString const pv)
{
    for (auto log_pv : m_log_pvs) {
        if (pv == log_pv)
            return true;
    }

    return false;
}

QList<QSharedPointer<PvSpace>> ChangeMirrorDialog::getPvSpaceList()
{
    QList<QSharedPointer<PvSpace>> list;
    const bool separate = LvmConfig::getMirrorLogsRequireSeparatePvs();
    const bool islvm = m_lv->isLvmMirror();
    const QList<PhysVol *> available_pvs = m_lv->getVg()->getPhysicalVolumes();
    
    if (m_lv->isMirror() && !m_change_log) {
        for (auto pv : available_pvs) {
            if (pv->isAllocatable() && (pv->getRemaining() > 0) && !pvHasImage(pv->getMapperName())) {
                if (separate && islvm) {                
                    if (!pvHasLog(pv->getMapperName()))
                        list << QSharedPointer<PvSpace>(new PvSpace(pv, pv->getRemaining(), pv->getContiguous()));
                } else {
                    list << QSharedPointer<PvSpace>(new PvSpace(pv, pv->getRemaining(), pv->getContiguous()));
                }
            }
        }
    } else if (m_change_log) {
        for (auto pv : available_pvs) {
            if (pv->isAllocatable() && (pv->getRemaining() > 0) && !pvHasLog(pv->getMapperName())) {
                if (separate) {                
                    if (!pvHasImage(pv->getMapperName()))
                        list << QSharedPointer<PvSpace>(new PvSpace(pv, pv->getRemaining(), pv->getContiguous()));
                } else {
                    list << QSharedPointer<PvSpace>(new PvSpace(pv, pv->getRemaining(), pv->getContiguous()));
                }
            }
        }
    } else {                              // not a mirror
        for (auto pv : available_pvs) {
            if (pv->isAllocatable() && (pv->getRemaining() > 0) && !pvHasImage(pv->getMapperName()))
                list << QSharedPointer<PvSpace>(new PvSpace(pv, pv->getRemaining(), pv->getContiguous()));
        }
    }

    return list;
}

/* Here we create a string based on all
   the options that the user chose in the
   dialog and feed that to "lvconvert" */

QStringList ChangeMirrorDialog::arguments()
{
    QStringList args;

    args << "lvconvert";

    if (!m_lv->isMirror()) {
        if (m_type_combo->currentIndex() == 1)
            args << "--type" << "raid1";
        else
            args << "--type" << "mirror" << "--regionsize" << "512k";
    }

    if (m_change_log || (!m_lv->isMirror() && m_type_combo->currentIndex() == 0)) {
        if (m_core_log_button->isChecked())
            args << "--mirrorlog" << "core";
        else if (m_mirrored_log_button->isChecked())
            args << "--mirrorlog" << "mirrored";
        else
            args << "--mirrorlog" << "disk";
    }

    if (!m_change_log) {
        args << "--mirrors" << QString("+%1").arg(m_add_mirrors_spin->value());

        if (m_stripe_spin->value() > 1) {
            args << "--stripes" <<  QString("%1").arg(m_stripe_spin->value());
            args << "--stripesize" << (m_stripe_size_combo->currentText()).remove("KiB").trimmed();
        }
    }

    args << "--background" << m_lv->getFullName();

    if (m_log_widget) {
        if (m_disk_log_button->isChecked()) {
            if (m_log_one->isChecked())
                args << m_log_one->getUnmungedText();
            else
                args << m_log_two->getUnmungedText();
        }
    } else {
        if (m_pv_box->getPolicy() <= ANYWHERE) // don't pass INHERITED_*
            args << "--alloc" << policyToString(m_pv_box->getEffectivePolicy());

        args << m_pv_box->getNames();
    }

    return args;
}


/* This function is supposed to find the space needed
   for a log of a new mirror. The formula is not reliable
   though and the --regionsize parameter can be ignored by
   the lvm tools.  */    

long long ChangeMirrorDialog::getNewLogSize()
{
    /*
        long long size = LvmConfig::getMirrorRegionSize();
        size = 1 + m_lv->getExtents() / (size * 8);
        return size * m_lv->getVg()->getExtentSize();
    */

    // reserve 4 megs ... and hope its enough
    return ((0x3fffff / m_lv->getVg()->getExtentSize()) + 1) * m_lv->getVg()->getExtentSize();
}

int ChangeMirrorDialog::getNewLogCount()
{
   const bool is_mirror = m_lv->isMirror();
   int count = 0;

    if (m_change_log || (!is_mirror && (m_type_combo->currentIndex() == 0))) {
        if (m_disk_log_button->isChecked())
            count = 1;
        else if (m_mirrored_log_button->isChecked())
            count = 2;
        else
            count = 0;
    }

    return count;
}

/* Enable or disable the OK button based on having
   enough physical volumes checked. At least one pv
   for each mirror leg and one or two for the log(s)
   if separate pvs are needed. We also total up the 
   space required. */

void ChangeMirrorDialog::resetOkButton()
{
    int new_stripe_count = 1;
    int total_stripes = 0;   //  stripes per mirror * added mirrors
    const int new_log_count = getNewLogCount();
    const bool is_lvm  = m_lv->isMirror() && !m_lv->isRaid();

    if (!m_change_log) {
        if (!validateStripeSpin()) {
            enableButtonOk(false);
            return;
        }

        if (m_stripe_spin->value() > 1)
            new_stripe_count = m_stripe_spin->value();

        total_stripes = m_add_mirrors_spin->value() * new_stripe_count;
    } else if (m_change_log && (m_lv->getLogCount() == 2)) {
        if (new_log_count == 2)
            enableButtonOk(false);
        else
            enableButtonOk(true);

        return;
    }

    if (is_lvm) {
        if (m_change_log && (m_lv->getLogCount() == new_log_count)) {
            enableButtonOk(false);
            return;
        } else if (!m_change_log && !(total_stripes > 0)) {
            enableButtonOk(false);
            return;
        }
    } else if (!is_lvm && total_stripes <= 0) {
        enableButtonOk(false);
        return;
    }

    QList <long long> available_pv_bytes;
    int logs = 0;
    if (!getAvailableByteList(available_pv_bytes, logs, total_stripes)) {
        enableButtonOk(false);
        return;
    }

    /* if there are not enough new mirror legs to pair with any unhandled
       new logs we put the logs on selected pvs and discard those pvs. */

    while (!available_pv_bytes.isEmpty() && logs > total_stripes) {
        available_pv_bytes.removeFirst(); 
        --logs;
    }

    if (available_pv_bytes.isEmpty() && logs > 0) {
        enableButtonOk(false);
        return;
    }

    QList <long long> stripe_pv_bytes;
    for (int x = 0; x < total_stripes; ++x) {
        if (x < logs)
            stripe_pv_bytes.append(0 - getNewLogSize());
        else
            stripe_pv_bytes.append(0);
    }

    if (total_stripes) {
        while (available_pv_bytes.size()) {
            qSort(available_pv_bytes);
            qSort(stripe_pv_bytes);
            stripe_pv_bytes[0] += available_pv_bytes.takeLast();
        }
        qSort(stripe_pv_bytes);

        // reserve one extent for raid metadata
        if ((m_type_combo && m_type_combo->currentIndex() == 1) || m_lv->isRaid()) {
            if ((stripe_pv_bytes[0] - 1) >= (m_lv->getSize() / new_stripe_count))
                enableButtonOk(true);
            else
                enableButtonOk(false);
        } else {
            if (stripe_pv_bytes[0] >= (m_lv->getSize() / new_stripe_count))
                enableButtonOk(true);
            else
                enableButtonOk(false);
        }

        return;
    } else {
        enableButtonOk(true);
        return;
    }

    enableButtonOk(false);
    return;
}

void ChangeMirrorDialog::enableTypeOptions(int index)
{
    if (index == 0) {
        m_log_box->setEnabled(true);
        if (m_stripe_box)  // The physical tab is not always defined
            m_stripe_box->setEnabled(true);
    } else {
        m_log_box->setEnabled(false);
        m_disk_log_button->setChecked(true);
        if (m_stripe_spin)
            m_stripe_spin->setValue(1);
        if (m_stripe_box)
            m_stripe_box->setEnabled(false);
    }
}

void ChangeMirrorDialog::setLogRadioButtons()
{
    if (m_change_log) {
        if (m_lv->getLogCount() == 2)
            m_mirrored_log_button->setChecked(true);
        else if (m_lv->getLogCount() == 1)
            m_disk_log_button->setChecked(true);
        else
            m_core_log_button->setChecked(true);
    } else if (!m_lv->isLvmMirror()) {
        m_disk_log_button->setChecked(true);
    }

    resetOkButton();
}

void ChangeMirrorDialog::commit()
{
    hide();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    ProcessProgress add_mirror(arguments());
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

bool ChangeMirrorDialog::validateStripeSpin()
{
    if (m_stripe_spin->value() > 1) {
        if (m_lv->getExtents() % m_stripe_spin->value()) {
            m_error_stack->setCurrentIndex(0);  // unworkable stripe count
            return false;
        } else {
            m_error_stack->setCurrentIndex(1);  // valid stripe count
            return true;
        }
    } else {
        m_error_stack->setCurrentIndex(1);
        return true;
    }
}

void ChangeMirrorDialog::enableLogWidget()
{
    if(m_mirrored_log_button->isChecked())
        m_log_stack->setCurrentIndex(0);
    else if (m_disk_log_button->isChecked())
        m_log_stack->setCurrentIndex(1);
    else
        m_log_stack->setCurrentIndex(2);
}


/* Generate a list of the space available on each selected pv 
   usable for a new mirror leg. If a pv is selected by the user 
   that cannot be used for a leg, it tries to put a log there.
   The unhandledLogs parameter returns the number of logs that
   will need to go with the legs. The stripes parameter is the 
   stripes per leg multiplied by the number of new legs.
   Returns true on success. */ 

bool ChangeMirrorDialog::getAvailableByteList(QList<long long> &byte_list, int &unhandledLogs, const int stripes)
{
    const AllocationPolicy policy = m_pv_box->getEffectivePolicy();
    const bool separate_logs = LvmConfig::getMirrorLogsRequireSeparatePvs();
    unhandledLogs = 0;

    int logs_added = getNewLogCount() - m_lv->getLogCount();
    if (logs_added < 0)
        logs_added = 0; 

    if ((policy == CONTIGUOUS) || separate_logs) {  // contiguous allocation also implies separate logs

        for (auto pv_name : m_pv_box->getNames()) {
            if (!pvHasImage(pv_name) && !pvHasLog(pv_name)) { 
                for (auto available_space : m_space_list) {
                    if (available_space->pv->getMapperName() == pv_name) {
                        if (policy == CONTIGUOUS)
                            byte_list << available_space->contiguous;
                        else
                            byte_list << available_space->normal;
                    }
                }
            }
        }
        
        qSort(byte_list);
        while (!byte_list.isEmpty() && byte_list[0] <= 0)
            byte_list.removeAt(0);

        while (logs_added > 0 && byte_list.size()) {
            byte_list.removeFirst();
            --logs_added;
        }

        if (logs_added > 0)  // Too many logs for the selected pvs, so just bail out
            return false;

        if (policy == CONTIGUOUS) {
            while (byte_list.size() > stripes)  
                byte_list.removeFirst();
        }
    } else {
        
        for (auto pv_name : m_pv_box->getNames()) {
            if (pvHasImage(pv_name) && !pvHasLog(pv_name)) {
                --logs_added;
            } else if (!pvHasImage(pv_name)) { 
                for (auto available_space : m_space_list) {
                    if (available_space->pv->getMapperName() == pv_name)
                        byte_list << available_space->normal;
                }
            }
        }
        
        qSort(byte_list);
        while (!byte_list.isEmpty() && byte_list[0] <= 0)
            byte_list.removeAt(0);

        if (logs_added <= 0)
            unhandledLogs = 0;
        else
            unhandledLogs = logs_added;
        
        if (unhandledLogs > byte_list.size())
            return false;
    }
    
    return true;
}
