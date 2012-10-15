/*
 *
 *
 * Copyright (C) 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "lvcreatebase.h"

#include "misc.h"
#include "mountentry.h"
#include "processprogress.h"
#include "sizeselectorbox.h"

#include <math.h>

#include <KConfigSkeleton>
#include <KGlobal>
#include <KLineEdit>
#include <KLocale>
#include <KMessageBox>
#include <KTabWidget>

#include <QCheckBox>
#include <QDebug>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>



LvCreateDialogBase::LvCreateDialogBase(bool extend, bool snap, bool thin,
                                       QString name, QString pool, QWidget *parent):
    KDialog(parent), m_extend(extend)
{

    KConfigSkeleton skeleton;
    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", m_use_si_units, false);
    
    QWidget *const main_widget = new QWidget();
    QVBoxLayout *const layout = new QVBoxLayout();
    
    bool show_name_tag = true;
    bool show_persistent = true;
    bool show_ro = true;
    bool show_zero = true;
    bool show_skip_sync = true;
    bool show_monitor = true;
    bool show_misc = true;

    if (extend) {
        show_name_tag = false;
        show_persistent = false;
        show_ro = false;
        show_zero = false;
        show_skip_sync = false;
        show_monitor = false;
    } else if (snap && !thin) {
        show_zero = false;
        show_skip_sync = false;
    } else if (thin) {
        show_ro = false;
        show_zero = false;
        show_skip_sync = false;
        show_monitor = false;
        show_misc = false;
    }

    m_tab_widget = new KTabWidget(this);
    m_tab_widget->addTab(createGeneralTab(show_name_tag, show_ro, show_zero, show_misc),  i18nc("The standard common options", "General"));
    m_tab_widget->addTab(createAdvancedTab(show_persistent, show_skip_sync, show_monitor), i18nc("Less used, dangerous or complex options", "Advanced options"));

    QLabel *const banner1_label = new QLabel();
    banner1_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(banner1_label);

    if (!extend) {
        if (snap) {
            if (thin) {
                banner1_label->setText(i18n("Create a thin snapshot of: %1", name));
                setCaption(i18n("Create A Thin Snapshot"));
            } else {
                banner1_label->setText(i18n("Create a snapshot of: %1", name));
                setCaption(i18n("Create A Snapshot"));
            }
        } else {
            if (thin) {
                banner1_label->setText(i18n("Create a new thin volume"));
                setCaption(i18n("Create A New Thin Volume"));
            } else {
                banner1_label->setText(i18n("Create a new logical volume"));
                setCaption(i18n("Create A New Logical Volume"));
            }
        }
    } else {
        banner1_label->setText(i18n("Extend volume: %1", name));
        setCaption(i18n("Extend Volume"));
    }

    if (thin) {
        QLabel *const banner2_label = new QLabel(i18n("Pool: %1", pool));
        banner2_label->setAlignment(Qt::AlignCenter);
        layout->addWidget(banner2_label);
    }

    layout->addSpacing(5);
    layout->addWidget(m_tab_widget);
    main_widget->setLayout(layout);
    setMainWidget(main_widget);

    makeConnections();
}

void LvCreateDialogBase::makeConnections()
{
    connect(this, SIGNAL(okClicked()),
            this, SLOT(commit()));

    connect(m_persistent_box, SIGNAL(toggled(bool)),
            this, SLOT(resetOkButton()));

    connect(m_major_edit, SIGNAL(textEdited(QString)),
            this, SLOT(resetOkButton()));

    connect(m_minor_edit, SIGNAL(textEdited(QString)),
            this, SLOT(resetOkButton()));
}

QWidget* LvCreateDialogBase::createGeneralTab(bool showNameTag, bool showRo, bool showZero, bool showMisc)
{

    QWidget *const general_tab = new QWidget(this);
    QHBoxLayout *const layout = new QHBoxLayout;
    general_tab->setLayout(layout);

    m_general_layout = new QVBoxLayout();
    layout->addStretch();
    layout->addLayout(m_general_layout);
    layout->addStretch();

    QGroupBox *const volume_box = new QGroupBox();
    QVBoxLayout *const volume_layout = new QVBoxLayout;
    volume_box->setLayout(volume_layout);

    m_general_layout->addWidget(volume_box);

    if (showNameTag) {
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
    } else {
        m_name_edit = NULL;
        m_tag_edit = NULL;
    }

    m_size_selector = NULL;


    QGroupBox *const misc_box = new QGroupBox();
    QVBoxLayout *const misc_layout = new QVBoxLayout();
    misc_box->setLayout(misc_layout);

    m_readonly_check = new QCheckBox(i18n("Set read only"));
    misc_layout->addWidget(m_readonly_check);
    
    m_zero_check = new QCheckBox(i18n("Write zeros at volume start"));
    misc_layout->addWidget(m_zero_check);
    misc_layout->addStretch();
    
    if (!showRo) {
        m_readonly_check->hide();
    }

    if (!showZero) {
        m_zero_check->hide();
    }

    if (showZero && showRo) {


        // connect them here ....

    }

    m_general_layout->addWidget(misc_box);
    if (!showMisc)
        misc_box->hide();

    return general_tab;
}

QWidget* LvCreateDialogBase::createAdvancedTab(bool showPersistent, bool showSkipSync, bool showMonitor)
{
    QWidget *const advanced_tab = new QWidget(this); 

    QHBoxLayout *const advanced_layout = new QHBoxLayout;
    advanced_tab->setLayout(advanced_layout);
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

    if (!showSkipSync)
        m_skip_sync_check->hide();

    if (!showMonitor)
        m_monitor_check->hide();


    m_udevsync_check = new QCheckBox(i18n("Synchronize with udev"));
    m_udevsync_check->setChecked(true);
    layout->addWidget(m_udevsync_check);

    if (showPersistent) {
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
    } else {
        m_persistent_box = NULL;
        m_minor_edit = NULL;
        m_major_edit = NULL;
    }
        
    layout->addStretch();

    return advanced_tab;
}

void LvCreateDialogBase::setReadOnly(bool ro)
{
    m_readonly_check->setChecked(ro);
}

void LvCreateDialogBase::setSkipSync(bool skip)
{
    m_skip_sync_check->setChecked(skip);
}

void LvCreateDialogBase::setZero(bool zero)
{
    m_zero_check->setChecked(zero);
}

void LvCreateDialogBase::setMonitor(bool monitor)
{
    m_monitor_check->setChecked(monitor);
}

void LvCreateDialogBase::enableZero(bool zero)
{
    m_zero_check->setEnabled(zero);
}

void LvCreateDialogBase::enableSkipSync(bool skip)
{
    m_skip_sync_check->setEnabled(skip);
}


void LvCreateDialogBase::enableMonitor(bool monitor)
{
    m_monitor_check->setEnabled(monitor);
}

void LvCreateDialogBase::setUdev(bool udev)
{
    m_udevsync_check->setChecked(udev);
}

bool LvCreateDialogBase::getMonitor()
{
    return m_monitor_check->isChecked();
}

bool LvCreateDialogBase::getUdev()
{
    return m_udevsync_check->isChecked();
}

bool LvCreateDialogBase::isValid()
{
    bool valid = false;

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

    if (m_persistent_box->isChecked() && !(valid_major && valid_minor)) {
        valid = false;
    } else if (m_size_selector == NULL) {
        valid = true;
    } else if (m_size_selector->isValid() && valid_name) {
        if (m_extend) {
            if (m_size_selector->getCurrentSize() > m_size_selector->getMinimumSize())
                valid = true;
            else
                valid = false;
        } else {
            if (m_size_selector->getCurrentSize() > 0)
                valid = true;
            else
                valid = false;
        }
    } else {
        valid = false;
    }

    return valid;
}

bool LvCreateDialogBase::getReadOnly()
{
    return m_readonly_check->isChecked();
}

bool LvCreateDialogBase::getZero()
{
    return m_zero_check->isChecked();
}

QString LvCreateDialogBase::getMajor()
{
    return m_major_edit->text();
}

QString LvCreateDialogBase::getMinor()
{
    return m_minor_edit->text();
}

long long LvCreateDialogBase::getSize()
{
    return m_size_selector->getCurrentSize();
}

QString LvCreateDialogBase::getName()
{
    return m_name_edit->text();
}

QString LvCreateDialogBase::getTag()
{
    return m_tag_edit->text();
}

bool LvCreateDialogBase::getPersistent()
{
    return m_persistent_box->isChecked();
}

void LvCreateDialogBase::initializeSizeSelector(long long extentSize, long long currentExtents, long long maxExtents)
{
    m_size_selector = new SizeSelectorBox(extentSize, currentExtents,
                                          maxExtents, currentExtents,
                                          true, false);

    m_general_layout->insertWidget(1, m_size_selector);

    connect(m_size_selector, SIGNAL(stateChanged()),
            this, SLOT(resetOkButton()));
}

void LvCreateDialogBase::setMaxExtents(long long max)
{
    m_size_selector->setConstrainedMax(max);
}
