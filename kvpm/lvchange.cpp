/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013, 2014, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "lvchange.h"

#include "allocationpolicy.h"
#include "logvol.h"
#include "masterlist.h"
#include "processprogress.h"
#include "volgroup.h"

#include <KLocalizedString>

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QTabWidget>
#include <QVBoxLayout>



LVChangeDialog::LVChangeDialog(LogVol *const volume, QWidget *parent) :
    KvpmDialog(parent),
    m_lv(volume)
{
    setCaption(i18n("Change Logical Volume Attributes"));

    QWidget *const dialog_body = new QWidget();
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout;

    QLabel *const name = new QLabel(i18n("Change volume: %1", m_lv->getName()));
    name->setAlignment(Qt::AlignCenter);
    layout->addWidget(name);
    layout->addSpacing(10);
    layout->addStretch();

    QTabWidget *const tab_widget = new QTabWidget();
    layout->addWidget(tab_widget);
    tab_widget->addTab(buildGeneralTab(),  i18nc("The standard or basic options", "General"));
    tab_widget->addTab(buildAdvancedTab(), i18nc("Less used or complex options", "Advanced"));
    layout->addStretch();
    dialog_body->setLayout(layout);

    connect(m_available_check,   SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_ro_check,          SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_refresh_check,     SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_udevsync_check,    SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_persistent_check,  SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_deltag_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(resetOkButton()));
    connect(m_tag_edit,     SIGNAL(textChanged(QString)), this, SLOT(resetOkButton()));
    connect(m_polling_box,       SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_poll_button,       SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_nopoll_button,     SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_devnum_box,        SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_available_check, SIGNAL(stateChanged(int)), this , SLOT(refreshAndAvailableCheck()));
    connect(m_refresh_check,   SIGNAL(stateChanged(int)), this , SLOT(refreshAndAvailableCheck()));

    resetOkButton();
}

QWidget *LVChangeDialog::buildGeneralTab()
{
    QWidget *const tab = new QWidget();
    QVBoxLayout *const layout = new QVBoxLayout();
    tab->setLayout(layout);

    QGroupBox *const general_group = new QGroupBox();
    QVBoxLayout *const general_layout = new QVBoxLayout();
    general_group->setLayout(general_layout);
    layout->addWidget(general_group);

    m_available_check = new QCheckBox(i18n("Make volume available (active)"));
    m_ro_check        = new QCheckBox(i18n("Make volume read only"));
    m_refresh_check   = new QCheckBox(i18n("Refresh volume metadata"));
    general_layout->addWidget(m_available_check);
    general_layout->addWidget(m_ro_check);
    general_layout->addWidget(m_refresh_check);
    
    m_available_check->setChecked(m_lv->isActive());
    m_ro_check->setChecked(!m_lv->isWritable());

    if (m_lv->isMounted() || m_lv->isCowSnap())
        m_available_check->setEnabled(false);

    QGroupBox *const tag_group = new QGroupBox(i18n("Volume Tags"));
    layout->addWidget(tag_group);
    QHBoxLayout *const add_tag_layout = new QHBoxLayout();
    QHBoxLayout *const del_tag_layout = new QHBoxLayout();
    QVBoxLayout *const tag_group_layout = new QVBoxLayout();
    tag_group_layout->addLayout(add_tag_layout);
    tag_group_layout->addLayout(del_tag_layout);
    tag_group->setLayout(tag_group_layout);
    QLabel *const add_tag_label = new QLabel(i18n("Add new tag:"));
    add_tag_layout->addWidget(add_tag_label);
    m_tag_edit = new QLineEdit();
    add_tag_label->setBuddy(m_tag_edit);
    QRegExp rx("[0-9a-zA-Z_\\.+-]*");
    QRegExpValidator *tag_validator = new QRegExpValidator(rx, m_tag_edit);
    m_tag_edit->setValidator(tag_validator);
    add_tag_layout->addWidget(m_tag_edit);
    QLabel *const del_tag_label = new QLabel(i18n("Remove tag:"));
    del_tag_layout->addWidget(del_tag_label);
    m_deltag_combo = new QComboBox();
    del_tag_label->setBuddy(m_deltag_combo);
    m_deltag_combo->setEditable(false);
    QStringList tags = m_lv->getTags();
    for (int x = 0; x < tags.size(); x++)
        m_deltag_combo->addItem(tags[x]);
    m_deltag_combo->insertItem(0, QString(""));
    m_deltag_combo->setCurrentIndex(0);
    del_tag_layout->addWidget(m_deltag_combo);

    if (!m_lv->isThinVolume()) {
        m_policy_combo = new PolicyComboBox(m_lv->getPolicy(), m_lv->getVg()->getPolicy());
        general_layout->addWidget(m_policy_combo);

        connect(m_policy_combo, SIGNAL(policyChanged(AllocationPolicy)), 
                this, SLOT(resetOkButton()));
    } else {
        m_policy_combo = nullptr;
    }

    return tab;
}

QWidget *LVChangeDialog::buildAdvancedTab()
{
    QWidget *const tab = new QWidget();
    QVBoxLayout *const layout = new QVBoxLayout();
    tab->setLayout(layout);

    QGroupBox *const sync_box = new QGroupBox();
    layout->addWidget(sync_box);
    QVBoxLayout *const sync_layout = new QVBoxLayout();
    sync_box->setLayout(sync_layout);
    m_udevsync_check = new QCheckBox(i18n("Synchronize with udev"));
    m_udevsync_check->setChecked(true);
    sync_layout->addWidget(m_udevsync_check);

    m_polling_box = new QGroupBox(i18n("Change Volume Polling"));
    m_polling_box->setCheckable(true);
    m_polling_box->setChecked(false);
    layout->addWidget(m_polling_box);
    QVBoxLayout *const poll_layout = new QVBoxLayout();
    m_polling_box->setLayout(poll_layout);
    m_poll_button = new QRadioButton(i18n("Start polling"));
    m_poll_button->setChecked(true);
    poll_layout->addWidget(m_poll_button);
    m_nopoll_button = new QRadioButton(i18n("Stop polling"));
    poll_layout->addWidget(m_nopoll_button);

    m_dmeventd_box = new QGroupBox(i18n("Change dmeventd Monitoring"));
    m_dmeventd_box->setCheckable(true);
    m_dmeventd_box->setChecked(false);

    QVBoxLayout *const mirror_layout = new QVBoxLayout();
    m_dmeventd_box->setLayout(mirror_layout);
    m_monitor_button = new QRadioButton(i18n("Monitor with dmeventd"));
    m_nomonitor_button = new QRadioButton(i18n("Do not monitor"));
    m_ignore_button  = new QRadioButton(i18n("Ignore dmeventd"));
    m_monitor_button->setChecked(true);
    mirror_layout->addWidget(m_monitor_button);
    mirror_layout->addWidget(m_nomonitor_button);
    mirror_layout->addWidget(m_ignore_button);
    layout->addWidget(m_dmeventd_box);

    if (m_lv->isCowSnap() || m_lv->isMirror() || m_lv->isRaid()) {
        connect(m_dmeventd_box,     SIGNAL(clicked()), this, SLOT(resetOkButton()));
        connect(m_monitor_button,   SIGNAL(clicked()), this, SLOT(resetOkButton()));
        connect(m_nomonitor_button, SIGNAL(clicked()), this, SLOT(resetOkButton()));
        connect(m_ignore_button,    SIGNAL(clicked()), this, SLOT(resetOkButton()));
    } else {
        m_dmeventd_box->setEnabled(false);
        m_dmeventd_box->hide();
    }

    layout->addStretch();

    m_devnum_box = new QGroupBox(i18n("Change Kernel Device Numbers"));
    m_devnum_box->setCheckable(true);
    QVBoxLayout *const devnum_layout = new QVBoxLayout();
    m_devnum_box->setLayout(devnum_layout);
    QHBoxLayout *const major_layout = new QHBoxLayout();
    QHBoxLayout *const minor_layout = new QHBoxLayout();
    m_persistent_check = new QCheckBox(i18n("Use persistent device numbers"));
    devnum_layout->addWidget(m_persistent_check);
    devnum_layout->addLayout(major_layout);
    devnum_layout->addLayout(minor_layout);

    m_major_edit = new QLineEdit(QString("%1").arg(m_lv->getMajorDevice()));
    QLabel *const major_label = new QLabel(i18n("Major number: "));
    major_label->setBuddy(m_major_edit);
    major_layout->addWidget(major_label);
    major_layout->addWidget(m_major_edit);

    m_minor_edit = new QLineEdit(QString("%1").arg(m_lv->getMinorDevice()));
    QLabel *const minor_label = new QLabel(i18n("Minor number: "));
    minor_label->setBuddy(m_minor_edit);
    minor_layout->addWidget(minor_label);
    minor_layout->addWidget(m_minor_edit);

    layout->addWidget(m_devnum_box);
    m_persistent_check->setChecked(m_lv->isPersistent());
    m_devnum_box->setChecked(false);

    return tab;
}

QStringList LVChangeDialog::arguments()
{
    QStringList args, temp;

    args << "lvchange" << "--yes"; // answer yes to any question

    if (!m_lv->isCowSnap()) {
        if (m_available_check->isChecked() && (!m_lv->isActive())) {
            args << "--available" << "y";

            if ( MasterList::isLvmVersionEqualOrGreater("2.02.109") )
                args << "-K";

        } else if ((!m_available_check->isChecked()) && (m_lv->isActive())) {
            args << "--available" << "n";
        }
    }

    if (m_ro_check->isChecked() && m_lv->isWritable())
        args << "--permission" << "r";
    else if ((!m_ro_check->isChecked()) && (!m_lv->isWritable()))
        args << "--permission" << "rw";

    if (m_refresh_check->isChecked())
        args << "--refresh";

    if (m_lv->isCowSnap() || m_lv->isMirror() || m_lv->isRaid()) {
        if (m_dmeventd_box->isChecked()) {
            if (m_monitor_button->isChecked())
                args << "--monitor" << "y";
            else if (m_nomonitor_button->isChecked())
                args << "--monitor" << "n";
            else
                args << "--ignoremonitoring";
        }
    }

    if (m_devnum_box->isChecked()) {
        if (m_persistent_check->isChecked()) {
            args << "--force" << "--persistent" << "y";
            args << "--major" << m_major_edit->text();
            args << "--minor" << m_minor_edit->text();
        } else {
            args << "--force" << "--persistent" << "n";
            args << "--major" << m_major_edit->text();
            args << "--minor" << m_minor_edit->text();
        }
    }

    if (m_polling_box->isChecked()) {
        if (m_poll_button->isChecked())
            args << "--poll" << "y";
        else
            args << "--poll" << "n";
    }

    if (!m_udevsync_check->isChecked())
        args << "--noudevsync";

    if (m_deltag_combo->currentIndex())
        args << "--deltag" << m_deltag_combo->currentText();
    if (!(m_tag_edit->text()).isEmpty())
        args << "--addtag" << m_tag_edit->text();

    if (m_policy_combo) {
        if (m_lv->getPolicy() != m_policy_combo->getPolicy())
            args << "--alloc" << policyToString(m_policy_combo->getPolicy());
    }

    args << m_lv->getFullName();

    return args;
}

void LVChangeDialog::commit()
{
    hide();
    ProcessProgress change_lv(arguments());
}

void LVChangeDialog::resetOkButton()
{

    if (m_available_check->isChecked()) {
        m_polling_box->setEnabled(true);
    } else {
        m_polling_box->setEnabled(false);
        m_polling_box->setChecked(false);
    }

    QStringList args = arguments();

    args.removeAt(args.indexOf(QString("--ignoremonitoring")));

    if (args.size() > 3)
        enableButtonOk(true);
    else
        enableButtonOk(false);
}


// metadata refresh and availability change can't happen at the same time

void LVChangeDialog::refreshAndAvailableCheck()
{
    if (!m_lv->isMounted() && !m_lv->isCowSnap()) {

        if (m_lv->isActive() == m_available_check->isChecked()) {
            m_refresh_check->setEnabled(true);
        } else {
            m_refresh_check->setEnabled(false);
            m_refresh_check->setChecked(false);
        }

        if (m_refresh_check->isChecked()) {
            m_available_check->setChecked(m_lv->isActive());
            m_available_check->setEnabled(false);
        } else {
            m_available_check->setEnabled(true);
        }
    }
}


