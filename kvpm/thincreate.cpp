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

#include "thincreate.h"

#include "fsextend.h"
#include "logvol.h"
#include "misc.h"
#include "mountentry.h"
#include "processprogress.h"
#include "sizeselectorbox.h"
#include "volgroup.h"

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



/* This class handles both the creation and extension of thin
   volumes and snapshots since both processes are so similar. */

ThinCreateDialog::ThinCreateDialog(LogVol *const pool, QWidget *parent):
    KDialog(parent),
    m_pool(pool)
{
    m_vg = m_pool->getVg();
    m_lv = NULL;
    m_extend = false;
    m_snapshot = false;
    m_bailout  = hasInitialErrors();
    m_fs_can_extend = false;

    if (!m_bailout)
        buildDialog();
}

ThinCreateDialog::ThinCreateDialog(LogVol *const volume, const bool snapshot, QWidget *parent):
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

void ThinCreateDialog::buildDialog()
{
    KConfigSkeleton skeleton;
    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", m_use_si_units, false);
    
    QWidget *const main_widget = new QWidget();
    QVBoxLayout *const layout = new QVBoxLayout();
    
    m_tab_widget = new KTabWidget(this);
    m_advanced_tab = createAdvancedTab();
    m_general_tab  = createGeneralTab();
    m_tab_widget->addTab(m_general_tab,  i18nc("The standard common options", "General"));
    m_tab_widget->addTab(m_advanced_tab, i18nc("Less used, dangerous or complex options", "Advanced options"));

    QLabel *const lv_name_label = new QLabel();
    lv_name_label->setAlignment(Qt::AlignCenter);
    
    if (m_extend) {
        setCaption(i18n("Extend Thin Volume"));
        lv_name_label->setText(i18n("<b>Extend volume: %1</b>", m_lv->getName()));
        layout->addWidget(lv_name_label);
    } else if (m_snapshot) {
        setCaption(i18n("Create Thin Snapshot"));
        lv_name_label->setText(i18n("<b>Create thin snapshot of: %1</b>", m_lv->getName()));
        layout->addWidget(lv_name_label);
    } else {
        setCaption(i18n("Create A New Thin Volume"));
        lv_name_label->setText(i18n("<b>Create a new thin volume"));
        QLabel *const pool_label = new QLabel();
        pool_label->setAlignment(Qt::AlignCenter);
        pool_label->setText(i18n("<b>Pool: %1</b>", m_pool->getName()));
        layout->addWidget(lv_name_label);
        layout->addWidget(pool_label);
    }

    layout->addSpacing(5);
    layout->addWidget(m_tab_widget);
    main_widget->setLayout(layout);
    setMainWidget(main_widget);

    makeConnections();
    resetOkButton();
}

void ThinCreateDialog::makeConnections()
{
    connect(this, SIGNAL(okClicked()),
            this, SLOT(commitChanges()));

    connect(m_persistent_box, SIGNAL(toggled(bool)),
            this, SLOT(resetOkButton()));

    connect(m_major_edit, SIGNAL(textEdited(QString)),
            this, SLOT(resetOkButton()));

    connect(m_minor_edit, SIGNAL(textEdited(QString)),
            this, SLOT(resetOkButton()));

    connect(m_size_selector, SIGNAL(stateChanged()),
            this, SLOT(resetOkButton()));
}

QWidget* ThinCreateDialog::createGeneralTab()
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

    if (!m_extend) {
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
    } else {
        volume_box->hide();
    }

    if (!m_snapshot) {
        if (m_extend) {
            m_size_selector = new SizeSelectorBox(m_vg->getExtentSize(), m_lv->getExtents(),
                                                  getLargestVolume() / m_vg->getExtentSize(),
                                                  m_lv->getExtents(), true, false);
        } else {
            m_size_selector = new SizeSelectorBox(m_vg->getExtentSize(), 0,
                                                  getLargestVolume() / m_vg->getExtentSize(),
                                                  0, true, false);
        }
        
        layout->addWidget(m_size_selector);
    } else {
        m_size_selector = NULL;
    }

    QGroupBox *const misc_box = new QGroupBox();
    QVBoxLayout *const misc_layout = new QVBoxLayout();
    misc_box->setLayout(misc_layout);

    m_readonly_check = new QCheckBox();
    m_readonly_check->setText(i18n("Set read only"));
    m_readonly_check->setChecked(false);
    m_readonly_check->setEnabled(false);
    misc_layout->addWidget(m_readonly_check);
    misc_layout->addStretch();

    layout->addWidget(misc_box);

    return general_tab;
}

QWidget* ThinCreateDialog::createAdvancedTab()
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
    layout->addWidget(m_monitor_check);

    if (m_snapshot) {
        m_monitor_check->setChecked(true);
        m_monitor_check->setEnabled(true);
    } else if (m_extend) {
        m_monitor_check->setChecked(false);
        m_monitor_check->setEnabled(false);
        m_monitor_check->hide();
    } else {
        m_monitor_check->setChecked(false);
        m_monitor_check->setEnabled(false);
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

void ThinCreateDialog::resetOkButton()
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

    if (m_persistent_box->isChecked() && !(valid_major && valid_minor)) {
        enableButtonOk(false);
    } else if (m_size_selector == NULL) {
        enableButtonOk(true);
    } else if (m_size_selector->isValid() && valid_name) {
        if (m_extend) {
            if (m_size_selector->getCurrentSize() > m_lv->getExtents())
                enableButtonOk(true);
            else
                enableButtonOk(false);
        } else {
            if (m_size_selector->getCurrentSize() > 0)
                enableButtonOk(true);
            else
                enableButtonOk(false);
        }
    } else {
        enableButtonOk(false);
    }
}

long long ThinCreateDialog::getLargestVolume()
{
    return 0x1000000000000;
}

/* Here we create a stringlist of arguments based on all
   the options that the user chose in the dialog. */

QStringList ThinCreateDialog::arguments()
{
    QString program_to_run;
    QStringList args;

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

    if (!m_extend) {
        if (m_readonly_check->isChecked())
            args << "--permission" << "r" ;
        else
            args << "--permission" << "rw" ;
    }

    if (m_monitor_check->isEnabled()) {
        args << "--monitor";
        if (m_monitor_check->isChecked())
            args << "y";
        else
            args << "n";
    }

    if (m_size_selector != NULL && !m_snapshot) {
        if (m_extend)
            args << "--size";
        else
            args << "--virtualsize";
 
        args << QString("%1b").arg(m_size_selector->getCurrentSize() * m_vg->getExtentSize());
    }

    if (!m_extend) {
        if (!(m_name_edit->text()).isEmpty())
            args << "--name" << m_name_edit->text();
    }

    if (!m_extend && !m_snapshot) {                     // create a thin volume
        program_to_run = "lvcreate";
        args << "--thin" << m_pool->getFullName();
    } else if (m_snapshot) {                            // create a thin snapshot
        program_to_run = "lvcreate";
        args << "--thin" << "--snapshot" << m_lv->getFullName();
    } else {                                            // extend the current volume
        program_to_run = "lvextend";
        args << m_lv->getFullName();
    }

    args.prepend(program_to_run);

    return args;
}

// This function checks for problems that would make showing this dialog pointless
// returns true if there are problems and is used to set m_bailout.

bool ThinCreateDialog::hasInitialErrors()
{
    if (m_extend) {

        const QString warning_message = i18n("If this volume has a filesystem or data, it will need to be extended <b>separately</b>. "
                                             "Currently, only the ext2, ext3, ext4, xfs, jfs, ntfs and reiserfs file systems are "
                                             "supported for extension. The correct executable for extension must also be present. ");



        //
        //
        // Change this to isCowOrigin() then test for open COW snaps only
        //
        //

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

        m_fs_can_extend = fs_can_extend(m_lv->getFilesystem());

        if (!(m_fs_can_extend || m_lv->isCowSnap())) {
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

bool ThinCreateDialog::bailout()
{
    return m_bailout;
}

void ThinCreateDialog::commitChanges()
{
    QStringList lvchange_args;
    hide();

    if (!m_extend) {
        ProcessProgress create_lv(arguments());
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

            ProcessProgress extend_origin(arguments());
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
            ProcessProgress extend_lv(arguments());
            if (!extend_lv.exitCode() && !m_lv->isCowSnap() && m_fs_can_extend)
                fs_extend(mapper_path, fs, m_lv->getMountPoints(), true);

            return;
        }
    }
}

