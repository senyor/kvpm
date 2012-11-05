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

#include "changemirror.h"

#include "logvol.h"
#include "misc.h"
#include "physvol.h"
#include "pvgroupbox.h"
#include "processprogress.h"
#include "volgroup.h"

#include <math.h>

#include <KApplication>
#include <KComboBox>
#include <KLocale>
#include <KIcon>
#include <KIntSpinBox>
#include <KTabWidget>

#include <QDebug>
#include <QEventLoop>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QStackedWidget>
#include <QVBoxLayout>



ChangeMirrorDialog::ChangeMirrorDialog(LogVol *const mirrorVolume, bool changeLog, QWidget *parent):
    KDialog(parent),
    m_change_log(changeLog),
    m_lv(mirrorVolume)
{
    if (changeLog) {
        if (mirrorVolume->isLvmMirrorLog() && mirrorVolume->isLvmMirrorLeg()) // mirrored mirror log
            m_lv = mirrorVolume->getParent()->getParent();
        else if (mirrorVolume->isLvmMirrorLog() || mirrorVolume->isLvmMirrorLeg())
            m_lv = mirrorVolume->getParent();

        if (m_lv->getName().contains("_mimagetmp_"))  // under conversion temp mirror
            m_lv = m_lv->getParent();
    }

    QList<LogVol *> children;
    const QString lv_name = m_lv->getName();
    const bool is_raid = (m_lv->isMirror() && m_lv->isRaid());
    const bool is_lvm = (m_lv->isMirror() && !m_lv->isRaid());

    setWindowTitle(i18n("Change Mirror"));

    QWidget *const main_widget = new QWidget();
    QVBoxLayout *const layout = new QVBoxLayout();

    QLabel  *const lv_name_label = new QLabel();

    if(m_change_log)
        lv_name_label->setText(i18n("<b>Change mirror log: %1</b>", m_lv->getName()));
    else if(is_lvm)
        lv_name_label->setText(i18n("<b>Change LVM mirror: %1</b>", m_lv->getName()));
    else if(is_lvm)
        lv_name_label->setText(i18n("<b>Change RAID 1 mirror: %1</b>", m_lv->getName()));
    else
        lv_name_label->setText(i18n("<b>Convert to a mirror: %1</b>", m_lv->getName()));

    lv_name_label->setAlignment(Qt::AlignCenter);
    m_tab_widget = new KTabWidget();
    layout->addWidget(lv_name_label);
    layout->addSpacing(5);
    layout->addWidget(m_tab_widget);
    main_widget->setLayout(layout);

    setMainWidget(main_widget);

    m_tab_widget->addTab(buildGeneralTab(is_raid, is_lvm),  i18nc("Common user options", "General"));

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
    } else
        m_pv_box = NULL;

    setLogRadioButtons();

    connect(m_disk_log_button, SIGNAL(toggled(bool)),
            this, SLOT(resetOkButton()));

    connect(m_core_log_button, SIGNAL(toggled(bool)),
            this, SLOT(resetOkButton()));
    
    connect(m_mirrored_log_button, SIGNAL(toggled(bool)),
            this, SLOT(resetOkButton()));

    connect(this, SIGNAL(okClicked()),
            this, SLOT(commitChanges()));
}

QWidget *ChangeMirrorDialog::buildGeneralTab(const bool isRaidMirror, const bool isLvmMirror)
{
    QWidget *const general = new QWidget;
    QHBoxLayout *const general_layout = new QHBoxLayout;
    QVBoxLayout *const center_layout = new QVBoxLayout;
    const bool is_mirror = (isRaidMirror || isLvmMirror);

    m_type_combo = new KComboBox();
    m_add_mirrors_spin = new KIntSpinBox(1, 10, 1, 1, this);

    general_layout->addStretch();
    general_layout->addLayout(center_layout);
    general_layout->addStretch();
        
    QGroupBox *const add_mirror_box = new QGroupBox();
    QLabel  *const type_label = new QLabel(i18n("Mirror type: "));
    
    QHBoxLayout *const type_layout = new QHBoxLayout;
    type_layout->addWidget(type_label);
    type_layout->addWidget(m_type_combo);
    
    QVBoxLayout *const add_mirror_box_layout = new QVBoxLayout;
    add_mirror_box_layout->addLayout(type_layout);
    add_mirror_box->setLayout(add_mirror_box_layout);
    center_layout->addStretch();
    center_layout->addWidget(add_mirror_box);

    QLabel  *const existing_label = new QLabel(i18n("Existing mirror legs: %1", m_lv->getMirrorCount()));
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
            m_type_combo->addItem("Standard LVM");
            m_type_combo->addItem("RAID 1");
            
            connect(m_type_combo, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(enableTypeOptions(int)));
        }
    }

    m_log_box = new QGroupBox(i18n("Mirror logging"));
    QVBoxLayout *const log_box_layout = new QVBoxLayout;
    m_core_log_button = new QRadioButton(i18n("Memory based log"));
    m_disk_log_button = new QRadioButton(i18n("Disk based log"));
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
        } else 
            m_disk_log_button->setChecked(true);
    }
    else
        m_log_box->hide();

    if (m_change_log && (m_lv->getLogCount() == 2)) {
        m_log_widget = buildLogWidget();
        center_layout->addWidget(m_log_widget);

        connect(m_disk_log_button, SIGNAL(toggled(bool)),
                this, SLOT(enableLogWidget()));
        
        connect(m_core_log_button, SIGNAL(toggled(bool)),
                this, SLOT(enableLogWidget()));
        
        connect(m_mirrored_log_button, SIGNAL(toggled(bool)),
                this, SLOT(enableLogWidget()));
    } else
        m_log_widget = NULL;

    center_layout->addStretch();
    general->setLayout(general_layout);

    return general;
}

QWidget *ChangeMirrorDialog::buildLogWidget()
{
    QGroupBox *const log = new QGroupBox("Log to remove");
    QVBoxLayout *const layout = new QVBoxLayout();

    QStringList names;
    QList<LogVol *> const logs = m_lv->getAllChildrenFlat();
    for (int x = logs.size() - 1; x >= 0; x--) {
        if (logs[x]->isLvmMirrorLog() && !logs[x]->isMirror())
            names.append(logs[x]->getPvNamesAll());
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

    layout->addWidget(m_log_one);
    layout->addWidget(m_log_two);
    log->setEnabled(false);
    log->setLayout(layout);

    return log;
}

QWidget *ChangeMirrorDialog::buildPhysicalTab(const bool isRaidMirror)
{
    QWidget *const physical = new QWidget;
    QList<PhysVol *> unused_pvs = m_lv->getVg()->getPhysicalVolumes();
    const QStringList pvs_in_use = getPvsInUse();

// pvs with a mirror leg or log already on them aren't
// suitable for another so we remove those here

    for (int x = unused_pvs.size() - 1; x >= 0; x--) {
        if (!unused_pvs[x]->isAllocatable() || unused_pvs[x]->getRemaining() <= 0) {
            unused_pvs.removeAt(x);
        } else {
            for (int y = 0; y < pvs_in_use.size() ; y++) {
                if (unused_pvs[x]->getName() == pvs_in_use[y]) {
                    unused_pvs.removeAt(x);
                    break;
                }
            }
        }
    }

    QVBoxLayout *const physical_layout = new QVBoxLayout();

    QList<long long> normal;
    QList<long long> contiguous;

    for (int x = 0; x < unused_pvs.size(); x++) {
            normal.append(unused_pvs[x]->getRemaining());
            contiguous.append(unused_pvs[x]->getContiguous());
    }

    m_pv_box = new PvGroupBox(unused_pvs, normal, contiguous, m_lv->getPolicy());
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
    
    m_stripe_size_combo = new KComboBox();
    for (int n = 2; (pow(2, n) * 1024) <= m_lv->getVg()->getExtentSize() ; n++) {
        m_stripe_size_combo->addItem(QString("%1").arg(pow(2, n)) + " KiB");
        m_stripe_size_combo->setItemData(n - 2, QVariant((int) pow(2, n)), Qt::UserRole);

        if ((n - 2) < 5) 
            m_stripe_size_combo->setCurrentIndex(n - 2);
    }
    
    QLabel *const stripe_size = new QLabel(i18n("Stripe Size: "));
    m_stripe_spin = new KIntSpinBox();
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
    stripe_error1->setPixmap(KIcon("dialog-warning").pixmap(32, 32));
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

/* The next function returns a list of physical volumes in
   use by the mirror as legs or logs. */

QStringList ChangeMirrorDialog::getPvsInUse()
{
    QList<LogVol *>  legs = m_lv->getAllChildrenFlat();
    QStringList pvs_in_use;

    if (m_lv->isMirror()) {
        for (int x = legs.size() - 1; x >= 0; x--) {

            if ((!legs[x]->isMirrorLeg() && !legs[x]->isLvmMirrorLog()))
                legs.removeAt(x);
            else
                pvs_in_use << legs[x]->getPvNamesAll();
        }
    } else {
        pvs_in_use << m_lv->getPvNamesAll();
    }

    return pvs_in_use;
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
            args << "--type" << "mirror";
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
    } else
        args << "--mirrors" << QString("+0");

    args << "--background" << m_lv->getFullName();

    if (m_log_widget != NULL) {
        if (m_log_widget->isEnabled()) {
            if (m_log_one->isChecked())
                args << m_log_one->getUnmungedText();
            else
                args << m_log_two->getUnmungedText();
        }
    } else {
        const AllocationPolicy policy = m_pv_box->getPolicy();

        if (policy != INHERITED)                          // "inherited" is what we get if we don't pass "--alloc" at all
            args << "--alloc" << policyToString(policy);  // passing "--alloc" "inherited" won't work                                                           
         args << m_pv_box->getNames();
    }

    return args;
}

    
int ChangeMirrorDialog::getNewLogCount()
{
   const bool is_mirror = m_lv->isMirror();
   int count;

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
   are needed. We also total up the space needed.   */

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

    QList <long long> stripe_pv_bytes;
    for (int x = 0; x < total_stripes; x++)
        stripe_pv_bytes.append(0);

    if (is_lvm) {
        if (m_change_log && (m_lv->getLogCount() == new_log_count)) {
            enableButtonOk(false);
            return;
        } else if (!m_change_log && !(total_stripes > 0)) {
            enableButtonOk(false);
            return;
        }
    } else if (!is_lvm && !((total_stripes > 0) || (m_lv->getLogCount() != new_log_count))) {
        enableButtonOk(false);
        return;
    }

    QList <long long> available_pv_bytes = m_pv_box->getRemainingSpaceList();;
    qSort(available_pv_bytes);

    AllocationPolicy policy = m_pv_box->getPolicy();

    if (policy == INHERITED)
        policy = m_lv->getVg()->getPolicy();

    if (policy == CONTIGUOUS) {

        int logs_added = new_log_count - m_lv->getLogCount();
        if (logs_added < 0)
            logs_added = 0; 

        while (available_pv_bytes.size() > total_stripes + logs_added)  
            available_pv_bytes.removeFirst();
    } 

    for (int x = m_lv->getLogCount(); x < new_log_count; x++) {
        if (available_pv_bytes.size())
            available_pv_bytes.removeFirst();
        else {
            enableButtonOk(false);
            return;
        }
    }

    if (total_stripes) {
        while (available_pv_bytes.size()) {
            qSort(available_pv_bytes);
            qSort(stripe_pv_bytes);
            stripe_pv_bytes[0] += available_pv_bytes.takeLast();
        }
        qSort(stripe_pv_bytes);

        if (stripe_pv_bytes[0] >= (m_lv->getSize() / new_stripe_count))
            enableButtonOk(true);
        else
            enableButtonOk(false);
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
        m_stripe_box->setEnabled(true);
    }
    else {
        m_log_box->setEnabled(false);
        m_disk_log_button->setChecked(true);
        m_stripe_spin->setValue(1);
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
    } else if (!m_lv->isLvmMirror())
        m_disk_log_button->setChecked(true);

    resetOkButton();
}

void ChangeMirrorDialog::commitChanges()
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
    if (m_disk_log_button->isChecked())
        m_log_widget->setEnabled(true);
    else
        m_log_widget->setEnabled(false);
}
