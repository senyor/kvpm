/*
 *
 *
 * Copyright (C) 2012, 2014, 2016 Benjamin Scott   <benscott@nwlink.com>
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

#include "mountentry.h"
#include "processprogress.h"
#include "sizeselectorbox.h"
#include "volgroup.h"

#include <cmath>

#include <KConfigSkeleton>
#include <KFormat>
#include <KLocalizedString>

#include <QCheckBox>
#include <QGroupBox>
#include <QIcon>
#include <QLabel>
#include <QTabWidget>
#include <QVBoxLayout>



LvCreateDialogBase::LvCreateDialogBase(const VolGroup *const vg, long long maxFsSize, 
                                       bool extend, bool snap, bool thin, bool ispool,
                                       QString name, QString pool, QWidget *parent):
    KvpmDialog(parent),
    m_vg(vg), 
    m_extend(extend),
    m_ispool(ispool),
    m_maxfs_size(maxFsSize)
{

    KConfigSkeleton skeleton;
    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", m_use_si_units, false);
    
    QWidget *const main_widget = new QWidget();
    QVBoxLayout *const layout = new QVBoxLayout();
    
    m_size_selector = nullptr;
    m_max_size = 0;

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
        show_zero = false;
        show_skip_sync = false;
        show_monitor = false;
    }

    m_tab_widget = new QTabWidget(this);
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
            } else if (ispool) {
                banner1_label->setText(i18n("Create a new thin pool"));
                setCaption(i18n("Create A New Thin Pool"));
            } else {
                banner1_label->setText(i18n("Create a new logical volume"));
                setCaption(i18n("Create A New Logical Volume"));
            }
        }
    } else {
        if (thin) {
            banner1_label->setText(i18n("Extend thin volume: %1", name));
            setCaption(i18n("Extend Thin Volume"));
        } else if (ispool) {
            banner1_label->setText(i18n("Extend thin pool: %1", name));
            setCaption(i18n("Extend Thin Pool"));
        } else {
            banner1_label->setText(i18n("Extend volume: %1", name));
            setCaption(i18n("Extend Volume"));
        }
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

    if (!extend) {
        connect(m_persistent_box, SIGNAL(toggled(bool)),
                this, SLOT(resetOkButton()));
        
        connect(m_major_edit, SIGNAL(textEdited(QString)),
                this, SLOT(resetOkButton()));
        
        connect(m_minor_edit, SIGNAL(textEdited(QString)),
                this, SLOT(resetOkButton()));
        
        connect(m_name_edit, SIGNAL(textEdited(QString)),
                this, SLOT(resetOkButton()));
        
        connect(m_tag_edit, SIGNAL(textEdited(QString)),
                this, SLOT(resetOkButton()));
    }
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
        m_name_edit = new QLineEdit();
        
        QRegExp rx("[0-9a-zA-Z_\\.][-0-9a-zA-Z_\\.]*");
        m_name_validator = new QRegExpValidator(rx, m_name_edit);
        m_name_edit->setValidator(m_name_validator);

        QLabel *const name_label = new QLabel(i18n("Volume name: "));
        if (m_ispool)
            name_label->setText(i18n("Pool name (required): "));

        name_label->setBuddy(m_name_edit);
        name_layout->addWidget(name_label);
        name_layout->addWidget(m_name_edit);
        volume_layout->insertLayout(0, name_layout);
        
        QHBoxLayout *const tag_layout = new QHBoxLayout();
        m_tag_edit = new QLineEdit();
        
        QRegExp rx2("[0-9a-zA-Z_\\.+-]*");
        m_tag_validator = new QRegExpValidator(rx2, m_tag_edit);
        m_tag_edit->setValidator(m_tag_validator);
        QLabel *const tag_label = new QLabel(i18n("Tag (optional): "));
        tag_label->setBuddy(m_tag_edit);
        tag_layout->addWidget(tag_label);
        tag_layout->addWidget(m_tag_edit);
        volume_layout->insertLayout(1, tag_layout);
    } else {
        m_name_edit = nullptr;
        m_tag_edit = nullptr;
        volume_box->hide();
    }

    QGroupBox *const misc_box = new QGroupBox();
    QVBoxLayout *const misc_layout = new QVBoxLayout();
    misc_box->setLayout(misc_layout);

    m_readonly_check = new QCheckBox(i18n("Set read only"));
    m_zero_check = new QCheckBox(i18n("Write zeros at volume start"));
    misc_layout->addWidget(m_readonly_check);
    misc_layout->addWidget(m_zero_check);
    
    if (!showRo)
        m_readonly_check->hide();

    if (!showZero)
        m_zero_check->hide();

    if (showZero && showRo) {
        connect(m_readonly_check, SIGNAL(toggled(bool)),
                this, SLOT(zeroReadOnlyEnable()));

        connect(m_zero_check, SIGNAL(toggled(bool)),
                this, SLOT(zeroReadOnlyEnable()));
    }

    m_extend_fs_check = new QCheckBox(i18n("Extend filesystem with volume"));
    misc_layout->addWidget(m_extend_fs_check);

    if (!m_extend || m_ispool) {
        m_extend_fs_check->setChecked(false);
        m_extend_fs_check->setEnabled(false);
        m_extend_fs_check->hide();
    } else {
        if (m_maxfs_size < 0) {
            m_extend_fs_check->setChecked(false);
            m_extend_fs_check->setEnabled(false);
        } else {
            m_extend_fs_check->setChecked(true);

            connect(m_extend_fs_check, SIGNAL(toggled(bool)),
                    this, SIGNAL(extendFs()));
        }
    }

    misc_layout->addSpacing(10);

    m_warning_widget = createWarningWidget();
    misc_layout->addWidget(m_warning_widget);
    m_warning_widget->hide();
    
    m_current_label    = new QLabel();
    m_extend_label     = new QLabel();
    m_maxfs_size_label = new QLabel();
    m_stripes_label    = new QLabel();
    m_max_size_label   = new QLabel();
    misc_layout->addWidget(m_current_label);
    misc_layout->addWidget(m_extend_label);
    misc_layout->addWidget(m_max_size_label);
    misc_layout->addWidget(m_maxfs_size_label);
    misc_layout->addWidget(m_stripes_label);
    if (!m_extend) {
        m_current_label->hide();
        m_extend_label->hide();
        m_maxfs_size_label->hide();
    }
    misc_layout->addStretch();

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
    QGroupBox *const options_box = new QGroupBox();
    QVBoxLayout *const layout = new QVBoxLayout;
    QVBoxLayout *const options_layout = new QVBoxLayout;
    options_box->setLayout(options_layout);
    layout->addWidget(options_box);
    advanced_layout->addStretch();
    advanced_layout->addLayout(layout);
    advanced_layout->addStretch();
    m_monitor_check = new QCheckBox(i18n("Monitor with dmeventd"));
    m_skip_sync_check = new QCheckBox(i18n("Skip initial synchronization of mirror"));
    m_skip_sync_check->setChecked(false);
    options_layout->addWidget(m_monitor_check);
    options_layout->addWidget(m_skip_sync_check);

    if (!showSkipSync)
        m_skip_sync_check->hide();

    if (!showMonitor)
        m_monitor_check->hide();

    m_udevsync_check = new QCheckBox(i18n("Synchronize with udev"));
    m_udevsync_check->setChecked(true);
    options_layout->addWidget(m_udevsync_check);

    if (showPersistent) {
        QVBoxLayout *const persistent_layout   = new QVBoxLayout;
        QHBoxLayout *const minor_number_layout = new QHBoxLayout;
        QHBoxLayout *const major_number_layout = new QHBoxLayout;
        m_minor_edit = new QLineEdit();
        m_major_edit = new QLineEdit();
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
        m_persistent_box = nullptr;
        m_minor_edit = nullptr;
        m_major_edit = nullptr;
    }
        
    layout->addStretch();
    advanced_layout->addStretch();

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


// Returns true if the selected size is smaller than the current
// size when extending or less than zero for a new volume.
// NOTE: returns false for size zero new volumes and volumes
// of the exact same size when extending! The function isValid()
// will return false for those conditions however.

bool LvCreateDialogBase::isLow()
{ 
    if (m_size_selector != nullptr) {
        if (m_size_selector->getNewSize() == -1) {  // if the size selector line edit is empty
            return false;
        } else if (!m_size_selector->isValid()) {
            if (m_extend) {
                if (m_size_selector->getNewSize() >= m_size_selector->getMinimumSize())
                    return false;
                else
                    return true;
            } else {
                if (m_size_selector->getNewSize() >= 0)
                    return false;
                else
                    return true;
            }
        }
    }

    return false;
}

bool LvCreateDialogBase::isValid()
{
    bool valid = false;

    bool valid_name  = true;
    bool valid_tag   = true;
    bool valid_persistent = true;

    if (!m_extend) {
        int pos = 0;
        QString name = m_name_edit->text();

        if (m_name_validator->validate(name, pos) == QValidator::Acceptable && name != "." && name != "..")
            valid_name = true;
        else if (name.isEmpty() && !m_ispool)
            valid_name = true;
        else
            valid_name = false;

        QString tag = m_tag_edit->text();

        if (m_tag_validator->validate(tag, pos) == QValidator::Acceptable)
            valid_tag = true;
        else if (tag.isEmpty() && !m_ispool)
            valid_tag = true;
        else
            valid_tag = false;
        
        QString major = m_major_edit->text();
        QString minor = m_minor_edit->text();
        const QValidator *const major_validator = m_major_edit->validator();
        const QValidator *const minor_validator = m_minor_edit->validator();
        const bool valid_major = (major_validator->validate(major, pos) == QValidator::Acceptable); 
        const bool valid_minor = (minor_validator->validate(minor, pos) == QValidator::Acceptable); 

        if (m_persistent_box->isChecked() && !(valid_major && valid_minor))
            valid_persistent = false;
        else
            valid_persistent = true;
    }

    if (!valid_persistent || !valid_name || !valid_tag) {
        valid = false;
    } else if (m_size_selector == nullptr) {
        valid = true;
    } else if (m_size_selector->isValid()) {
        if (m_extend) {
            if (m_size_selector->getNewSize() > m_size_selector->getMinimumSize())
                valid = true;
            else
                valid = false;
        } else {
            if (m_size_selector->getNewSize() > 0)
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

bool LvCreateDialogBase::getSkipSync()
{
    return m_skip_sync_check->isChecked();
}

QString LvCreateDialogBase::getMajor()
{
    return m_major_edit->text();
}

QString LvCreateDialogBase::getMinor()
{
    return m_minor_edit->text();
}
long long LvCreateDialogBase::getSelectorExtents()
{
    return m_size_selector->getNewSize();
}

QString LvCreateDialogBase::getName()
{
    if (m_name_edit != nullptr)
        return m_name_edit->text();
    else 
        return QString();
}

QString LvCreateDialogBase::getTag()
{
    if (m_tag_edit != nullptr)
        return m_tag_edit->text();
    else 
        return QString();
}

bool LvCreateDialogBase::getPersistent()
{
    return m_persistent_box->isChecked();
}

void LvCreateDialogBase::initializeSizeSelector(long long extentSize, long long currentExtents, long long maxExtents)
{

    if (currentExtents >= m_maxfs_size / extentSize) {
        m_maxfs_size = -1;
        m_extend_fs_check->setChecked(false);
        m_extend_fs_check->setEnabled(false);
    }

    m_size_selector = new SizeSelectorBox(extentSize, currentExtents,
                                          maxExtents, currentExtents,
                                          true, false);

    m_general_layout->insertWidget(1, m_size_selector);

    connect(m_size_selector, SIGNAL(stateChanged()),
            this, SLOT(resetOkButton()));

    connect(m_size_selector, SIGNAL(stateChanged()),
            this, SLOT(setSizeLabels()));

    KFormat::BinaryUnitDialect dialect;

    if (m_use_si_units)
        dialect = KFormat::MetricBinaryDialect;
    else
        dialect = KFormat::IECBinaryDialect;

    m_current_label->setText(i18n("Current size: %1", KFormat().formatByteSize(currentExtents * extentSize, 1, dialect)));
}

void LvCreateDialogBase::setSelectorMaxExtents(long long max)
{
    if (m_size_selector != nullptr)
        m_size_selector->setConstrainedMax(max);
}

void LvCreateDialogBase::zeroReadOnlyEnable()
{
    disconnect(m_readonly_check, SIGNAL(toggled(bool)),
               this, SLOT(zeroReadOnlyEnable()));
    
    disconnect(m_zero_check, SIGNAL(toggled(bool)),
               this, SLOT(zeroReadOnlyEnable()));
    
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

    resetOkButton();
    
    connect(m_readonly_check, SIGNAL(toggled(bool)),
            this, SLOT(zeroReadOnlyEnable()));
    
    connect(m_zero_check, SIGNAL(toggled(bool)),
            this, SLOT(zeroReadOnlyEnable()));
}

void LvCreateDialogBase::setPhysicalTab(QWidget *const tab)
{
    m_tab_widget->insertTab(1, tab, i18n("Physical layout"));
}

QWidget* LvCreateDialogBase::createWarningWidget()
{
    QWidget *const widget = new QWidget();
    QHBoxLayout *const layout = new QHBoxLayout();

    QLabel *const exclamation = new QLabel(); 
    exclamation->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-warning")).pixmap(32, 32));
    m_warning_label = new QLabel();
    layout->addWidget(exclamation);
    layout->addWidget(m_warning_label);
    layout->addStretch();
    widget->setLayout(layout);

    return widget;
}

void LvCreateDialogBase::setWarningLabel(const QString message)
{
    m_warning_widget->show();
    m_warning_label->setText(message);
}

void LvCreateDialogBase::clearWarningLabel()
{
    m_warning_label->setText("");
    m_warning_widget->hide();
}

const VolGroup* LvCreateDialogBase::getVg()
{
    return m_vg;
}

void LvCreateDialogBase::setSizeLabels()
{
    KFormat::BinaryUnitDialect dialect;

    if (m_use_si_units)
        dialect = KFormat::MetricBinaryDialect;
    else
        dialect = KFormat::IECBinaryDialect;

    if (m_size_selector != nullptr) {

        const long long extend = m_size_selector->getNewSize() - m_size_selector->getMinimumSize(); 
        const long long extent_size = m_vg->getExtentSize();

        if (m_size_selector->usingBytes()) {
            if (extend >= 0)
                m_extend_label->setText(i18n("Increasing size: +%1", KFormat().formatByteSize(extend * extent_size, 1, dialect)));
            else
                m_extend_label->setText(i18n("Increasing size: %1", KFormat().formatByteSize(extend * extent_size, 1, dialect)));
        } else {
            if (extend >= 0)
                m_extend_label->setText(i18n("Increasing extents: +%1", extend));
            else
                m_extend_label->setText(i18n("Increasing extents: %1", extend));
        }

        if (m_maxfs_size < 0) {
            if (m_size_selector->usingBytes()) {
                m_max_size_label->setText(i18n("Maximum volume size: %1", KFormat().formatByteSize(m_max_size, 1, dialect)));
            } else {
                m_max_size_label->setText(i18n("Maximum volume extents: %1", m_max_size / extent_size));
            }
        } else {
            if (m_size_selector->usingBytes()) {
                m_max_size_label->setText(i18n("Maximum volume size: %1", KFormat().formatByteSize(m_max_size, 1, dialect)));
                m_maxfs_size_label->setText(i18n("Maximum filesystem size: %1", KFormat().formatByteSize(m_maxfs_size, 1, dialect)));
            } else {
                m_max_size_label->setText(i18n("Maximum volume extents: %1", m_max_size / extent_size));
                m_maxfs_size_label->setText(i18n("Maximum filesystem extents: %1", m_maxfs_size / extent_size));
            }
        }
    } else {
        m_extend_label->hide();
        m_max_size_label->hide();
        m_maxfs_size_label->hide();
    }
}

void LvCreateDialogBase::setInfoLabels(VolumeType type, int stripes, int mirrors, long long maxSize)
{
    m_max_size = maxSize;
    setSizeLabels();

    if (m_size_selector != nullptr) {
        m_stripes_label->show();

        if (type == LINEAR) {
            if (stripes > 1)
                m_stripes_label->setText(i18n("(with %1 stripes)", stripes));
            else
                m_stripes_label->setText(i18n("(linear volume)"));
        } else if (type == LVMMIRROR) {
            if (stripes > 1)
                m_stripes_label->setText(i18n("(LVM2 mirror with %1 legs and %2 stripes)", mirrors, stripes));
            else
                m_stripes_label->setText(i18n("(LVM2 mirror with %1 legs)", mirrors));
        } else if (type == RAID1) {
            m_stripes_label->setText(i18n("(RAID 1 mirror with %1 legs)", mirrors));
        } else if (type == RAID4) {
            m_stripes_label->setText(i18n("(RAID 4 with %1 stripes + 1 parity)", stripes));
        } else if (type == RAID5) {
            m_stripes_label->setText(i18n("(RAID 5 with %1 stripes + 1 parity)", stripes));
        } else if (type == RAID5) {
            m_stripes_label->setText(i18n("(RAID 6 with %1 stripes + 2 parity)", stripes));
        }
    } else {
        m_stripes_label->hide();
    }
}

long long LvCreateDialogBase::getMaxFsSize()
{
    return m_maxfs_size;

}

bool  LvCreateDialogBase::getExtendFs()
{
    return m_extend_fs_check->isChecked();
}

