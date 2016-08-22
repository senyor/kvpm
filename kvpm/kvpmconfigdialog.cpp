/*
 *
 *
 * Copyright (C) 2009, 2010, 2011, 2012, 2013, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "kvpmconfigdialog.h"

#include "executablefinder.h"

#include <KColorButton>
#include <KConfigSkeleton>
#include <KEditListWidget>
#include <KLocalizedString>
#include <KSeparator>

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QString>
#include <QTableWidget>
#include <QTabWidget>
#include <QVBoxLayout>




KvpmConfigDialog::KvpmConfigDialog(QWidget *parent,
                                   const QString name,
                                   KConfigSkeleton *const skeleton,
                                   ExecutableFinder *const executableFinder)
    : KConfigDialog(parent, name, skeleton),
      m_skeleton(skeleton),
      m_executable_finder(executableFinder)
{
    setFaceType(KPageDialog::Auto);
    button(QDialogButtonBox::Help)->setDefault(true);

    addPage(generalPage(), i18nc("The standard common options", "General"), QString("configure"));
    addPage(colorsPage(), i18n("Colors"), QString("color-picker"));
    addPage(programsPage(), i18n("Programs"), QString("applications-system"));
}

KvpmConfigDialog::~KvpmConfigDialog()
{
    m_skeleton->deleteLater();
}

QTabWidget *KvpmConfigDialog::generalPage()
{
    QTabWidget *const tabwidget = new QTabWidget;
    tabwidget->insertTab(1, treesTab(), i18n("Tree Views"));
    tabwidget->insertTab(1, propertiesTab(), i18n("Property Panels"));

    return tabwidget;
}

QWidget *KvpmConfigDialog::treesTab()
{
    QWidget *const trees = new QWidget;
    QVBoxLayout *const layout = new QVBoxLayout();
    QLabel *const banner = new QLabel(i18n("<b>Set columns to show in tables and tree views</b>"));
    banner->setAlignment(Qt::AlignCenter);
    layout->addWidget(banner);

    QHBoxLayout *const horizontal_layout = new QHBoxLayout();
    horizontal_layout->addWidget(deviceGroup());
    horizontal_layout->addWidget(logicalGroup());
    horizontal_layout->addWidget(physicalGroup());
    horizontal_layout->addWidget(allGroup());

    layout->addLayout(horizontal_layout);
    trees->setLayout(layout);

    return trees;
}

QWidget *KvpmConfigDialog::propertiesTab()
{
    QWidget *const properties = new QWidget();
    QVBoxLayout *const layout = new QVBoxLayout();
    QLabel *const banner = new QLabel(i18n("<b>Set information to show in property panels</b>"));
    banner->setAlignment(Qt::AlignCenter);
    layout->addWidget(banner);

    QHBoxLayout *const horizontal_layout = new QHBoxLayout();
    horizontal_layout->addWidget(devicePropertiesGroup());
    horizontal_layout->addWidget(lvPropertiesGroup());
    horizontal_layout->addWidget(pvPropertiesGroup());

    layout->addLayout(horizontal_layout);
    properties->setLayout(layout);

    return properties;
}

QTabWidget *KvpmConfigDialog::programsPage()
{
    m_executables_table = new QTableWidget();
    m_executables_table->setColumnCount(2);

    const QStringList headers = QStringList() << i18n("Program name") << i18n("Full path");

    m_executables_table->setHorizontalHeaderLabels(headers);

    QTabWidget  *const programs = new QTabWidget;
    QVBoxLayout *const programs1_layout = new QVBoxLayout();
    QVBoxLayout *const programs2_layout = new QVBoxLayout();
    QWidget *const programs1 = new QWidget();
    QWidget *const programs2 = new QWidget();
    programs1->setLayout(programs1_layout);
    programs2->setLayout(programs2_layout);

    const QStringList default_entries = QStringList() << "/sbin/"
                                        << "/usr/sbin/"
                                        << "/bin/"
                                        << "/usr/bin/"
                                        << "/usr/local/bin/"
                                        << "/usr/local/sbin/";

    static QStringList search_entries;
    m_skeleton->setCurrentGroup("SystemPaths");
    m_skeleton->addItemStringList("SearchPath", search_entries, default_entries);

    m_edit_list = new KEditListWidget();
    m_edit_list->setObjectName("kcfg_SearchPath");
    QLabel *search_label = new QLabel(i18n("<b>Set the search path for support programs</b>"));
    search_label->setAlignment(Qt::AlignCenter);
    programs1_layout->addWidget(search_label);
    programs1_layout->addWidget(m_edit_list);

    programs->insertTab(1, programs1, "Search path");
    m_edit_list->insertStringList(search_entries);

    programs2_layout->addWidget(m_executables_table);
    programs->insertTab(1, programs2, "Executables");

    fillExecutablesTable();

    return programs;
}

void KvpmConfigDialog::updateSettings()
{
    QStringList search = m_edit_list->items();  // system paths to search for binaries
    search.removeDuplicates();

    m_edit_list->clear();

    for (int x = 0; x < search.size(); x++) {
        if (!search[x].endsWith('/'))
            search[x].append('/');
    }

    m_edit_list->insertStringList(search);

    KConfigDialog::updateSettings();

    m_executable_finder->reload(search);
    fillExecutablesTable();
}

void KvpmConfigDialog::fillExecutablesTable()
{
    QTableWidgetItem *table_item = nullptr;

    const QStringList all_names = m_executable_finder->getAllNames();
    const QStringList all_paths = m_executable_finder->getAllPaths();
    const QStringList not_found = m_executable_finder->getNotFound();

    m_executables_table->clearContents();
    m_executables_table->setRowCount(all_names.size() + not_found.size());

    for (int x = 0; x < not_found.size(); x++) {
        table_item = new QTableWidgetItem(not_found[x]);
        m_executables_table->setItem(x, 0, table_item);

        table_item = new QTableWidgetItem(QIcon::fromTheme(QStringLiteral("dialog-error")), "Not Found");
        m_executables_table->setItem(x, 1, table_item);
    }

    // these lists should be the same length, but just in case...

    for (int x = 0; (x < all_names.size()) && (x < all_paths.size()); x++) {

        table_item = new QTableWidgetItem(all_names[x]);
        m_executables_table->setItem(x + not_found.size(), 0, table_item);

        table_item = new QTableWidgetItem(all_paths[x]);
        m_executables_table->setItem(x + not_found.size(), 1, table_item);

    }
    m_executables_table->resizeColumnsToContents();
    m_executables_table->setAlternatingRowColors(true);
    m_executables_table->verticalHeader()->hide();
}

QGroupBox *KvpmConfigDialog::deviceGroup()
{
    QGroupBox *const device_group = new QGroupBox(i18n("Device Tree"));
    QVBoxLayout *const device_layout = new QVBoxLayout();
    device_group->setLayout(device_layout);

    static bool device_column;
    static bool partition_column;
    static bool capacity_column;
    static bool devremaining_column;
    static bool usage_column;
    static bool group_column;
    static bool flags_column;
    static bool mount_column;
    static bool expand_parts;

    m_skeleton->setCurrentGroup("DeviceTreeColumns"); // dt == device tree
    m_skeleton->addItemBool("dt_device",      device_column,       true);
    m_skeleton->addItemBool("dt_partition",   partition_column,    true);
    m_skeleton->addItemBool("dt_capacity",    capacity_column,     true);
    m_skeleton->addItemBool("dt_remaining",   devremaining_column, true);
    m_skeleton->addItemBool("dt_usage",       usage_column, true);
    m_skeleton->addItemBool("dt_group",       group_column, true);
    m_skeleton->addItemBool("dt_flags",       flags_column, true);
    m_skeleton->addItemBool("dt_mount",       mount_column, true);
    m_skeleton->addItemBool("dt_expandparts", expand_parts, true);

    QCheckBox *const device_check      = new QCheckBox(i18n("Device name"));
    QCheckBox *const partition_check   = new QCheckBox(i18n("Partition type"));
    QCheckBox *const capacity_check    = new QCheckBox(i18n("Capacity"));
    QCheckBox *const devremaining_check = new QCheckBox(i18n("Remaining space"));
    QCheckBox *const usage_check       = new QCheckBox(i18n("Usage of device"));
    QCheckBox *const group_check       = new QCheckBox(i18n("Volume group"));
    QCheckBox *const flags_check       = new QCheckBox(i18n("Partition flags"));
    QCheckBox *const mount_check       = new QCheckBox(i18n("Mount point"));
    QCheckBox *const expandparts_check = new QCheckBox(i18n("Expand device tree"));

    device_check->setObjectName("kcfg_dt_device");
    partition_check->setObjectName("kcfg_dt_partition");
    capacity_check->setObjectName("kcfg_dt_capacity");
    devremaining_check->setObjectName("kcfg_dt_remaining");
    usage_check->setObjectName("kcfg_dt_usage");
    group_check->setObjectName("kcfg_dt_group");
    flags_check->setObjectName("kcfg_dt_flags");
    mount_check->setObjectName("kcfg_dt_mount");
    expandparts_check->setObjectName("kcfg_dt_expandparts");

    device_check->setToolTip(i18n("Show the path to the device, /dev/sda1 for example."));
    partition_check->setToolTip(i18n("Show the type of partition, 'extended' for example."));
    capacity_check->setToolTip(i18n("Show the storage capacity in megabytes, gigabytes or terabytes."));
    devremaining_check->setToolTip(i18n("Show the remaining storage in megabytes, gigabytes or terabytes."));
    usage_check->setToolTip(i18n("Show how the partition is being used. Usually the type of filesystem, such as ext4, \n"
                                 "swap space or as a physical volume."));
    group_check->setToolTip(i18n("If the partition is a physical volume this column shows the volume group it is in."));
    flags_check->setToolTip(i18n("Show any flags, such a 'boot.'"));
    mount_check->setToolTip(i18n("Show the mount point if the partition has a mounted filesystem."));
    expandparts_check->setToolTip(i18n("This determines if all partitions of all devices get shown at start up. \n "
                                       "The user can still expand or collapse the items by clicking on them."));

    device_layout->addWidget(device_check);
    device_layout->addWidget(partition_check);
    device_layout->addWidget(capacity_check);
    device_layout->addWidget(devremaining_check);
    device_layout->addWidget(usage_check);
    device_layout->addWidget(group_check);
    device_layout->addWidget(flags_check);
    device_layout->addWidget(mount_check);
    device_layout->addWidget(new KSeparator(Qt::Horizontal));
    QLabel *const label = new QLabel(i18n("At start up:"));
    label->setAlignment(Qt::AlignCenter);
    device_layout->addWidget(label);
    device_layout->addWidget(expandparts_check);
    device_layout->addStretch();

    return device_group;
}

QGroupBox *KvpmConfigDialog::physicalGroup()
{
    QGroupBox *const physical_group = new QGroupBox(i18n("Physical Volume Table"));
    QVBoxLayout *const physical_layout = new QVBoxLayout();
    physical_group->setLayout(physical_layout);

    static bool name_column;
    static bool size_column;
    static bool remaining_column;
    static bool used_column;
    static bool state_column;
    static bool allocate_column;
    static bool tags_column;
    static bool lvnames_column;

    m_skeleton->setCurrentGroup("PhysicalTreeColumns"); // pt == physical tree
    m_skeleton->addItemBool("pt_name",      name_column,      true);
    m_skeleton->addItemBool("pt_size",      size_column,      true);
    m_skeleton->addItemBool("pt_remaining", remaining_column, true);
    m_skeleton->addItemBool("pt_used",      used_column,      false);
    m_skeleton->addItemBool("pt_state",     state_column,     false);
    m_skeleton->addItemBool("pt_allocate",  allocate_column,  true);
    m_skeleton->addItemBool("pt_tags",      tags_column,      true);
    m_skeleton->addItemBool("pt_lvnames",   lvnames_column,   true);

    QCheckBox *const name_check  = new QCheckBox(i18n("Volume name"));
    QCheckBox *const size_check  = new QCheckBox(i18n("Size"));
    QCheckBox *const remaining_check  = new QCheckBox(i18n("Remaining space"));
    QCheckBox *const used_check  = new QCheckBox(i18n("Used space"));
    QCheckBox *const state_check = new QCheckBox(i18n("State"));
    QCheckBox *const allocate_check = new QCheckBox(i18n("Allocatable"));
    QCheckBox *const tags_check     = new QCheckBox(i18n("Tags"));
    QCheckBox *const lvnames_check  = new QCheckBox(i18n("Logical Volumes"));

    name_check->setObjectName("kcfg_pt_name");
    size_check->setObjectName("kcfg_pt_size");
    remaining_check->setObjectName("kcfg_pt_remaining");
    used_check->setObjectName("kcfg_pt_used");
    state_check->setObjectName("kcfg_pt_state");
    allocate_check->setObjectName("kcfg_pt_allocate");
    tags_check->setObjectName("kcfg_pt_tags");
    lvnames_check->setObjectName("kcfg_pt_lvnames");

    name_check->setToolTip(i18n("Show the path to the device, /dev/sda1 for example."));
    size_check->setToolTip(i18n("Show the storage capacity in megabytes, gigabytes or terabytes."));
    remaining_check->setToolTip(i18n("Show the remaining storage in megabytes, gigabytes or terabytes."));
    used_check->setToolTip(i18n("Show the used storage in megabytes, gigabytes or terabytes."));
    state_check->setToolTip(i18n("Show the state, either 'active' or 'inactive.'"));
    allocate_check->setToolTip(i18n("Shows whether or not the physical volume can have its extents allocated."));
    tags_check->setToolTip(i18n("List any tags associated with the physical volume."));
    lvnames_check->setToolTip(i18n("List any logical volumes associated with the physical volume."));

    physical_layout->addWidget(name_check);
    physical_layout->addWidget(size_check);
    physical_layout->addWidget(remaining_check);
    physical_layout->addWidget(used_check);
    physical_layout->addWidget(state_check);
    physical_layout->addWidget(allocate_check);
    physical_layout->addWidget(tags_check);
    physical_layout->addWidget(lvnames_check);
    physical_layout->addStretch();

    return physical_group;
}

QGroupBox *KvpmConfigDialog::logicalGroup()
{
    QGroupBox *const logical_group = new QGroupBox(i18n("Logical volume tree"));
    QVBoxLayout *const logical_layout = new QVBoxLayout();
    logical_group->setLayout(logical_layout);

    static bool volume_column;
    static bool size_column;
    static bool remaining_column;
    static bool type_column;
    static bool filesystem_column;
    static bool stripes_column;
    static bool stripesize_column;
    static bool snapmove_column;
    static bool state_column;
    static bool access_column;
    static bool tags_column;
    static bool mountpoints_column;

    m_skeleton->setCurrentGroup("VolumeTreeColumns"); // vt == volume tree
    m_skeleton->addItemBool("vt_volume",      volume_column,      true);
    m_skeleton->addItemBool("vt_size",        size_column,        true);
    m_skeleton->addItemBool("vt_remaining",   remaining_column,   true);
    m_skeleton->addItemBool("vt_type",        type_column,        true);
    m_skeleton->addItemBool("vt_filesystem",  filesystem_column,  false);
    m_skeleton->addItemBool("vt_stripes",     stripes_column,     false);
    m_skeleton->addItemBool("vt_stripesize",  stripesize_column,  false);
    m_skeleton->addItemBool("vt_snapmove",    snapmove_column,    true);
    m_skeleton->addItemBool("vt_state",       state_column,       true);
    m_skeleton->addItemBool("vt_access",      access_column,      false);
    m_skeleton->addItemBool("vt_tags",        tags_column,        true);
    m_skeleton->addItemBool("vt_mountpoints", mountpoints_column, false);

    QCheckBox *const volume_check      = new QCheckBox(i18n("Volume name"));
    QCheckBox *const size_check        = new QCheckBox(i18n("Size"));
    QCheckBox *const remaining_check   = new QCheckBox(i18n("Remaining space"));
    QCheckBox *const type_check        = new QCheckBox(i18n("Volume type"));
    QCheckBox *const filesystem_check  = new QCheckBox(i18n("Filesystem type"));
    QCheckBox *const stripes_check     = new QCheckBox(i18n("Stripe count"));
    QCheckBox *const stripesize_check  = new QCheckBox(i18n("Stripe size"));
    QCheckBox *const snapmove_check    = new QCheckBox(i18n("(\%)Data/Copy"));
    QCheckBox *const state_check       = new QCheckBox(i18n("Volume state"));
    QCheckBox *const access_check      = new QCheckBox(i18n("Volume access"));
    QCheckBox *const tags_check        = new QCheckBox(i18n("Tags"));
    QCheckBox *const mountpoints_check = new QCheckBox(i18n("Mount point"));

    volume_check->setObjectName("kcfg_vt_volume");
    size_check->setObjectName("kcfg_vt_size");
    remaining_check->setObjectName("kcfg_vt_remaining");
    type_check->setObjectName("kcfg_vt_type");
    filesystem_check->setObjectName("kcfg_vt_filesystem");
    stripes_check->setObjectName("kcfg_vt_stripes");
    stripesize_check->setObjectName("kcfg_vt_stripesize");
    snapmove_check->setObjectName("kcfg_vt_snapmove");
    state_check->setObjectName("kcfg_vt_state");
    access_check->setObjectName("kcfg_vt_access");
    tags_check->setObjectName("kcfg_vt_tags");
    mountpoints_check->setObjectName("kcfg_vt_mountpoints");

    volume_check->setToolTip(i18n("Show the volume name."));
    size_check->setToolTip(i18n("Show the storage capacity in megabytes, gigabytes or terabytes."));
    remaining_check->setToolTip(i18n("Show the remaining storage in megabytes, gigabytes or terabytes."));
    type_check->setToolTip(i18n("Show the type of volume, 'mirror' or 'linear,' for example."));
    filesystem_check->setToolTip(i18n("Show the filesystem type on the volume, 'ext3' or 'swap,' for example."));
    stripes_check->setToolTip(i18n("Show the number of stripes on the volume, if it is striped"));
    stripesize_check->setToolTip(i18n("Show the size of the stripes, if it is striped"));
    snapmove_check->setToolTip(i18n("For a mirror, show the percentage of the mirror synced. \n"
                                    "For a snapshot, show the percentage of the snapshot used. \n"
                                    "For a pvmove, show the percentage of the move completed."));
    state_check->setToolTip(i18n("Show the state, 'active' or 'invalid,' for example."));
    access_check->setToolTip(i18n("Show access, either read only or read and write."));
    tags_check->setToolTip(i18n("List any tags associated with the volume"));
    mountpoints_check->setToolTip(i18n("Show the mount point if the partition has a mounted filesystem."));

    logical_layout->addWidget(volume_check);
    logical_layout->addWidget(size_check);
    logical_layout->addWidget(remaining_check);
    logical_layout->addWidget(type_check);
    logical_layout->addWidget(filesystem_check);
    logical_layout->addWidget(stripes_check);
    logical_layout->addWidget(stripesize_check);
    logical_layout->addWidget(snapmove_check);
    logical_layout->addWidget(state_check);
    logical_layout->addWidget(access_check);
    logical_layout->addWidget(tags_check);
    logical_layout->addWidget(mountpoints_check);
    logical_layout->addStretch();

    return logical_group;
}

QGroupBox *KvpmConfigDialog::allGroup()
{
    QGroupBox *const all_group = new QGroupBox(i18n("All trees and tables"));
    QGroupBox *const percent_group = new QGroupBox(i18n("Remaining and used space"));

    QVBoxLayout *const all_layout = new QVBoxLayout();
    QVBoxLayout *const percent_layout = new QVBoxLayout();
    all_group->setLayout(all_layout);
    percent_group->setLayout(percent_layout);

    static bool show_percent;
    static bool show_total;
    static bool show_both;
    static int fs_warn_percent;
    static int pv_warn_percent;

    m_skeleton->setCurrentGroup("AllTreeColumns");
    m_skeleton->addItemBool("show_percent", show_percent, false);
    m_skeleton->addItemBool("show_total",   show_total,   false);
    m_skeleton->addItemBool("show_both",    show_both,    true);
    m_skeleton->addItemInt("fs_warn",    fs_warn_percent, 10);
    m_skeleton->addItemInt("pv_warn",    pv_warn_percent,  0);

    QRadioButton *const percent_radio = new QRadioButton(i18n("Show percentage"));
    QRadioButton *const total_radio   = new QRadioButton(i18n("Show total"));
    QRadioButton *const both_radio    = new QRadioButton(i18n("Show both"));

    percent_radio->setObjectName("kcfg_show_percent");
    total_radio->setObjectName("kcfg_show_total");
    both_radio->setObjectName("kcfg_show_both");

    percent_layout->addWidget(total_radio);
    percent_layout->addWidget(percent_radio);
    percent_layout->addWidget(both_radio);
    percent_layout->addWidget(new KSeparator(Qt::Horizontal));

    QHBoxLayout *const warn_layout = new QHBoxLayout;
    QHBoxLayout *const fs_warn_layout = new QHBoxLayout;
    QHBoxLayout *const pv_warn_layout = new QHBoxLayout;
    warn_layout->addWidget(new QLabel(i18n("Show warning icon")));
    QLabel *const warn_icon = new QLabel;
    warn_icon->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-warning")).pixmap(16, 16));
    warn_layout->addWidget(warn_icon);
    percent_layout->addLayout(warn_layout);
    percent_layout->addWidget(new QLabel(i18n("when space falls to or below:")));

    QSpinBox *const fs_warn_spin = new QSpinBox;
    fs_warn_spin->setObjectName("kcfg_fs_warn");
    fs_warn_spin->setRange(0, 99);
    fs_warn_spin->setSingleStep(1);
    fs_warn_spin->setPrefix("% ");
    fs_warn_spin->setSpecialValueText(i18n("Never"));
    fs_warn_layout->addWidget(fs_warn_spin);
    QLabel *const fs_warn_label = new QLabel(i18n("on a filesystem"));
    fs_warn_label->setBuddy(fs_warn_spin);
    fs_warn_layout->addWidget(fs_warn_label);
    fs_warn_layout->addStretch();

    QSpinBox *const pv_warn_spin = new QSpinBox;
    pv_warn_spin->setObjectName("kcfg_pv_warn");
    pv_warn_spin->setRange(0, 99);
    pv_warn_spin->setSingleStep(1);
    pv_warn_spin->setPrefix("% ");
    pv_warn_spin->setSpecialValueText(i18n("Never"));
    pv_warn_layout->addWidget(pv_warn_spin);
    QLabel *const pv_warn_label = new QLabel(i18n("on a physical volume"));
    pv_warn_label->setBuddy(pv_warn_spin);
    pv_warn_layout->addWidget(pv_warn_label);
    pv_warn_layout->addStretch();

    percent_layout->addLayout(fs_warn_layout);
    percent_layout->addLayout(pv_warn_layout);
    all_layout->addWidget(percent_group);
    all_layout->addStretch();

    return all_group;
}

QGroupBox *KvpmConfigDialog::devicePropertiesGroup()
{
    QGroupBox *const properties = new QGroupBox(i18n("Device Properties Panel"));
    QVBoxLayout *const layout = new QVBoxLayout();

    static bool mountpoint;
    static bool fsuuid;
    static bool fslabel;

    m_skeleton->setCurrentGroup("DeviceProperties"); // dp == device properties
    m_skeleton->addItemBool("dp_mount",   mountpoint, true);
    m_skeleton->addItemBool("dp_fsuuid",  fsuuid,     false);
    m_skeleton->addItemBool("dp_fslabel", fslabel,    false);

    QCheckBox *const mp_check = new QCheckBox(i18n("Mount points"));
    mp_check->setToolTip(i18n("Show the filesystem mount points for the device"));
    mp_check->setObjectName("kcfg_dp_mount");
    QCheckBox *const fsuuid_check = new QCheckBox(i18n("Filesystem uuid"));
    fsuuid_check->setToolTip(i18n("Show the filesytem UUID"));
    fsuuid_check->setObjectName("kcfg_dp_fsuuid");
    QCheckBox *const fslabel_check = new QCheckBox(i18n("Filesystem label"));
    fslabel_check->setToolTip(i18n("Show the filesystem label"));
    fslabel_check->setObjectName("kcfg_dp_fslabel");

    layout->addWidget(mp_check);
    layout->addWidget(fsuuid_check);
    layout->addWidget(fslabel_check);
    layout->addStretch();
    properties->setLayout(layout);

    return properties;
}

QGroupBox *KvpmConfigDialog::pvPropertiesGroup()
{
    QGroupBox *const properties = new QGroupBox(i18n("Physical Volume Properties Panel"));
    QVBoxLayout *const layout = new QVBoxLayout();

    static bool mda;
    static bool pvuuid;

    m_skeleton->setCurrentGroup("PhysicalVolumeProperties");
    m_skeleton->addItemBool("pp_mda",  mda,    true);
    m_skeleton->addItemBool("pp_uuid", pvuuid, false);

    QCheckBox *const mda_check = new QCheckBox(i18n("Metadata areas"));
    mda_check->setToolTip(i18n("Show information about physical volume metadata"));
    mda_check->setObjectName("kcfg_pp_mda");
    QCheckBox *const pvuuid_check = new QCheckBox(i18n("Physical volume uuid"));
    pvuuid_check->setToolTip(i18n("Show the physical volume UUID"));
    pvuuid_check->setObjectName("kcfg_pp_uuid");

    layout->addWidget(mda_check);
    layout->addWidget(pvuuid_check);
    layout->addStretch();
    properties->setLayout(layout);

    return properties;
}

QGroupBox *KvpmConfigDialog::lvPropertiesGroup()
{
    QGroupBox *const properties = new QGroupBox(i18n("Logical Volume Properties Panel"));
    QVBoxLayout *const layout = new QVBoxLayout();

    static bool mp;
    static bool fsuuid;
    static bool fslabel;
    static bool lvuuid;

    m_skeleton->setCurrentGroup("LogicalVolumeProperties"); // lp == logical volume properties
    m_skeleton->addItemBool("lp_mount",   mp,      true);
    m_skeleton->addItemBool("lp_fsuuid",  fsuuid,  false);
    m_skeleton->addItemBool("lp_fslabel", fslabel, false);
    m_skeleton->addItemBool("lp_uuid",    lvuuid,  false);

    QCheckBox *const mp_check = new QCheckBox(i18n("Mount points"));
    mp_check->setToolTip(i18n("Show the filesystem mount points for the device"));
    mp_check->setObjectName("kcfg_lp_mount");
    QCheckBox *const fsuuid_check = new QCheckBox(i18n("Filesystem uuid"));
    fsuuid_check->setToolTip(i18n("Show the filesytem UUID"));
    fsuuid_check->setObjectName("kcfg_lp_fsuuid");
    QCheckBox *const fslabel_check = new QCheckBox(i18n("Filesystem label"));
    fslabel_check->setToolTip(i18n("Show the filesystem label"));
    fslabel_check->setObjectName("kcfg_lp_fslabel");
    QCheckBox *const lvuuid_check = new QCheckBox(i18n("Logical volume uuid"));
    lvuuid_check->setToolTip(i18n("Show the logical volume UUID"));
    lvuuid_check->setObjectName("kcfg_lp_uuid");

    layout->addWidget(mp_check);
    layout->addWidget(fsuuid_check);
    layout->addWidget(fslabel_check);
    layout->addWidget(lvuuid_check);
    properties->setLayout(layout);
    layout->addStretch();

    return properties;
}

QWidget *KvpmConfigDialog::colorsPage()
{
    QGroupBox *const page = new QGroupBox(i18n("Volume and Partition Colors"));
    QVBoxLayout *const layout = new QVBoxLayout();

    QHBoxLayout *const message_layout = new QHBoxLayout();
    QLabel *const message = new QLabel(i18n("The graphical display of volumes and partitions can use "
                                            "color to show additional information. They can be displayed " 
                                            "by type or by the type of filesystem on them"));

    message->setWordWrap(true);
    message_layout->addSpacing(50);
    message_layout->addWidget(message);
    layout->addLayout(message_layout);
    message_layout->addSpacing(50);
    layout->addSpacing(10);

    static int type_combo_index;
    m_skeleton->setCurrentGroup("General");
    m_skeleton->addItemInt("type_combo", type_combo_index, 0);

    QHBoxLayout *const combo_layout = new QHBoxLayout();
    QComboBox *const combo = new QComboBox();
    combo->setObjectName("kcfg_type_combo");

    combo->addItem(i18n("Use color graphics by volume or partition type"));
    combo->addItem(i18n("Use color graphics by filesystem type"));
    combo_layout->addStretch();
    combo_layout->addWidget(combo);
    combo_layout->addStretch();
    layout->addLayout(combo_layout);
    layout->addSpacing(10);

    m_color_stack = new QStackedWidget();
    m_color_stack->addWidget(typeColors());
    m_color_stack->addWidget(fsColors());

    QHBoxLayout *const mid_layout = new QHBoxLayout();
    mid_layout->addWidget(m_color_stack);
    mid_layout->addWidget(otherColors());
    showOtherButtons(combo->currentIndex());
    layout->addLayout(mid_layout);
    page->setLayout(layout);

    connect(combo, SIGNAL(currentIndexChanged(int)), 
            m_color_stack, SLOT(setCurrentIndex(int)));

    connect(combo, SIGNAL(currentIndexChanged(int)), 
            this, SLOT(showOtherButtons(int)));

    return page;
}

QWidget *KvpmConfigDialog::fsColors()
{
    static QColor ext2_color;
    static QColor ext3_color;
    static QColor ext4_color;
    static QColor btrfs_color;
    static QColor reiser_color;
    static QColor reiser4_color;
    static QColor msdos_color;
    static QColor jfs_color;
    static QColor xfs_color;
    static QColor hfs_color;
    static QColor ntfs_color;
    static QColor swap_color;
    static QColor physvol_color;
    static QColor none_color;

    m_skeleton->setCurrentGroup("FilesystemColors");
    m_skeleton->addItemColor("ext2",    ext2_color,    Qt::blue);
    m_skeleton->addItemColor("ext3",    ext3_color,    Qt::darkBlue);
    m_skeleton->addItemColor("ext4",    ext4_color,    Qt::cyan);
    m_skeleton->addItemColor("btrfs",   btrfs_color,   Qt::yellow);
    m_skeleton->addItemColor("reiser",  reiser_color,  Qt::red);
    m_skeleton->addItemColor("reiser4", reiser4_color, Qt::darkRed);
    m_skeleton->addItemColor("msdos",   msdos_color,   Qt::darkYellow);
    m_skeleton->addItemColor("jfs",     jfs_color,     Qt::magenta);
    m_skeleton->addItemColor("xfs",     xfs_color,     Qt::darkGreen);
    m_skeleton->addItemColor("hfs",     hfs_color,     Qt::darkMagenta);
    m_skeleton->addItemColor("ntfs",    ntfs_color,    Qt::darkGray);
    m_skeleton->addItemColor("none",    none_color,    Qt::black);
    m_skeleton->addItemColor("swap",    swap_color,    Qt::lightGray);
    m_skeleton->addItemColor("physvol", physvol_color, Qt::darkCyan);

    QWidget *const fscolors = new QWidget;
    QGridLayout *const layout = new QGridLayout();

    fscolors->setLayout(layout);

    KSeparator *const left_separator  = new KSeparator(Qt::Vertical);
    KSeparator *const right_separator = new KSeparator(Qt::Vertical);

    layout->addWidget(left_separator,  1, 4, 5, 1);
    layout->addWidget(right_separator, 1, 9, 5, 1);

    QLabel *const ext2_label = new QLabel("ext2");
    layout->addWidget(ext2_label, 1, 1, Qt::AlignRight);
    KColorButton *const ext2_button = new KColorButton();
    ext2_button->setObjectName("kcfg_ext2");
    layout->addWidget(ext2_button, 1, 2, Qt::AlignLeft);
    ext2_label->setBuddy(ext2_button);

    QLabel *const ext3_label = new QLabel("ext3");
    layout->addWidget(ext3_label, 2, 1, Qt::AlignRight);
    KColorButton *const ext3_button = new KColorButton();
    ext3_button->setObjectName("kcfg_ext3");
    layout->addWidget(ext3_button, 2, 2, Qt::AlignLeft);
    ext3_label->setBuddy(ext3_button);

    QLabel *const ext4_label = new QLabel("ext4");
    layout->addWidget(ext4_label, 3, 1, Qt::AlignRight);
    KColorButton *const ext4_button = new KColorButton();
    ext4_button->setObjectName("kcfg_ext4");
    layout->addWidget(ext4_button, 3, 2, Qt::AlignLeft);
    ext4_label->setBuddy(ext4_button);

    QLabel *const btrfs_label = new QLabel("btrfs");
    layout->addWidget(btrfs_label, 4, 1, Qt::AlignRight);
    KColorButton *const btrfs_button = new KColorButton();
    btrfs_button->setObjectName("kcfg_btrfs");
    layout->addWidget(btrfs_button, 4, 2, Qt::AlignLeft);
    btrfs_label->setBuddy(btrfs_button);

    QLabel *const reiser_label = new QLabel("reiser");
    layout->addWidget(reiser_label, 1, 6, Qt::AlignRight);
    KColorButton *const reiser_button = new KColorButton();
    reiser_button->setObjectName("kcfg_reiser");
    layout->addWidget(reiser_button, 1, 7, Qt::AlignLeft);
    reiser_label->setBuddy(reiser_button);

    QLabel *const reiser4_label = new QLabel("reiser4");
    layout->addWidget(reiser4_label, 2, 6, Qt::AlignRight);
    KColorButton *const reiser4_button = new KColorButton();
    reiser4_button->setObjectName("kcfg_reiser4");
    layout->addWidget(reiser4_button, 2, 7, Qt::AlignLeft);
    reiser4_label->setBuddy(reiser4_button);

    QLabel *const msdos_label = new QLabel("ms-dos");
    layout->addWidget(msdos_label, 3, 6, Qt::AlignRight);
    KColorButton *const msdos_button = new KColorButton();
    msdos_button->setObjectName("kcfg_msdos");
    layout->addWidget(msdos_button, 3, 7, Qt::AlignLeft);
    msdos_label->setBuddy(msdos_button);

    QLabel *const jfs_label = new QLabel("jfs");
    layout->addWidget(jfs_label, 1, 11, Qt::AlignRight);
    KColorButton *const jfs_button = new KColorButton();
    jfs_button->setObjectName("kcfg_jfs");
    layout->addWidget(jfs_button, 1, 12, Qt::AlignLeft);
    jfs_label->setBuddy(jfs_button);

    QLabel *const xfs_label = new QLabel("xfs");
    layout->addWidget(xfs_label, 2, 11, Qt::AlignRight);
    KColorButton *const xfs_button = new KColorButton();
    xfs_button->setObjectName("kcfg_xfs");
    layout->addWidget(xfs_button, 2, 12, Qt::AlignLeft);
    xfs_label->setBuddy(xfs_button);

    QLabel *const swap_label = new QLabel("linux \nswap");
    layout->addWidget(swap_label, 3, 11, Qt::AlignRight);
    KColorButton *const swap_button = new KColorButton();
    swap_button->setObjectName("kcfg_swap");
    layout->addWidget(swap_button, 3, 12, Qt::AlignLeft);
    swap_label->setBuddy(swap_button);

    QLabel *const none_label = new QLabel("unknown");
    layout->addWidget(none_label, 4, 6,  Qt::AlignRight);
    KColorButton *const none_button = new KColorButton();
    none_button->setObjectName("kcfg_none");
    layout->addWidget(none_button, 4, 7, Qt::AlignLeft);
    none_label->setBuddy(none_button);

    QLabel *const hfs_label = new QLabel("hfs");
    layout->addWidget(hfs_label, 4, 11,  Qt::AlignRight);
    KColorButton *const hfs_button = new KColorButton();
    hfs_button->setObjectName("kcfg_hfs");
    layout->addWidget(hfs_button, 4, 12, Qt::AlignLeft);
    hfs_label->setBuddy(hfs_button);

    QLabel *const ntfs_label = new QLabel("ntfs");
    layout->addWidget(ntfs_label, 5, 11,  Qt::AlignRight);
    KColorButton *const ntfs_button = new KColorButton();
    ntfs_button->setObjectName("kcfg_ntfs");
    layout->addWidget(ntfs_button, 5, 12, Qt::AlignLeft);
    ntfs_label->setBuddy(ntfs_button);

    QLabel *const physical_label = new QLabel("physical \nvolumes");
    layout->addWidget(physical_label, 5, 1,  Qt::AlignRight);
    KColorButton *const physical_button = new KColorButton();
    physical_button->setObjectName("kcfg_physvol");
    layout->addWidget(physical_button, 5, 2, Qt::AlignLeft);
    physical_label->setBuddy(physical_button);

    return fscolors;
}

QWidget *KvpmConfigDialog::typeColors()
{
    QWidget *const type = new QWidget;

    static QColor pvmove_color;
    static QColor mirror_color;
    static QColor raid1_color;
    static QColor raid456_color;
    static QColor cowsnap_color;
    static QColor invalid_color;
    static QColor other_color;
    static QColor linear_color;
    static QColor thinsnap_color;
    static QColor thinvol_color;
    static QColor inactive_color;

    m_skeleton->setCurrentGroup("VolumeTypeColors");
    m_skeleton->addItemColor("mirror",   mirror_color,   Qt::darkBlue);   // lvm type mirror
    m_skeleton->addItemColor("raid1",    raid1_color,    Qt::blue);
    m_skeleton->addItemColor("raid456",  raid456_color,  Qt::cyan);
    m_skeleton->addItemColor("thinvol",  thinvol_color,  Qt::lightGray);
    m_skeleton->addItemColor("invalid",  invalid_color,  Qt::red);
    m_skeleton->addItemColor("thinsnap", thinsnap_color, Qt::darkRed);
    m_skeleton->addItemColor("cowsnap",  cowsnap_color,  Qt::darkYellow);
    m_skeleton->addItemColor("linear",   linear_color,   Qt::darkCyan);
    m_skeleton->addItemColor("pvmove",   pvmove_color,   Qt::magenta);
    m_skeleton->addItemColor("other",    other_color,    Qt::yellow);
    m_skeleton->addItemColor("inactive", inactive_color, Qt::black);

    QGridLayout *const layout = new QGridLayout();
    type->setLayout(layout);

    KSeparator *const left_separator  = new KSeparator(Qt::Vertical);
    KSeparator *const right_separator = new KSeparator(Qt::Vertical);

    layout->addWidget(left_separator,  1, 4, 4, 1);
    layout->addWidget(right_separator, 1, 9, 4, 1);

    QLabel *const linear_label = new QLabel("linear");
    layout->addWidget(linear_label, 1, 1, Qt::AlignRight);
    KColorButton *const linear_button = new KColorButton();
    linear_button->setObjectName("kcfg_linear");
    layout->addWidget(linear_button, 1, 2, Qt::AlignLeft);
    linear_label->setBuddy(linear_button);

    QLabel *const raid1_label = new QLabel("RAID 1");
    layout->addWidget(raid1_label, 2, 1, Qt::AlignRight);
    KColorButton *const raid1_button = new KColorButton();
    raid1_button->setObjectName("kcfg_raid1");
    layout->addWidget(raid1_button, 2, 2, Qt::AlignLeft);
    raid1_label->setBuddy(raid1_button);

    QLabel *const raid456_label = new QLabel("RAID 4/5/6");
    layout->addWidget(raid456_label, 3, 1, Qt::AlignRight);
    KColorButton *const raid456_button = new KColorButton();
    raid456_button->setObjectName("kcfg_raid456");
    layout->addWidget(raid456_button, 3, 2, Qt::AlignLeft);
    raid456_label->setBuddy(raid456_button);

    QLabel *const cowsnap_label = new QLabel("snapshot");
    layout->addWidget(cowsnap_label, 4, 1, Qt::AlignRight);
    KColorButton *const cowsnap_button = new KColorButton();
    cowsnap_button->setObjectName("kcfg_cowsnap");
    layout->addWidget(cowsnap_button, 4, 2, Qt::AlignLeft);
    cowsnap_label->setBuddy(cowsnap_button);

    QLabel *const invalid_label = new QLabel("invalid snap");
    layout->addWidget(invalid_label, 5, 1, Qt::AlignRight);
    KColorButton *const invalid_button = new KColorButton();
    invalid_button->setObjectName("kcfg_invalid");
    layout->addWidget(invalid_button, 5, 2, Qt::AlignLeft);
    invalid_label->setBuddy(invalid_button);

    QLabel *const mirror_label = new QLabel("lvm mirror");
    layout->addWidget(mirror_label, 6, 1, Qt::AlignRight);
    KColorButton *const mirror_button = new KColorButton();
    mirror_button->setObjectName("kcfg_mirror");
    layout->addWidget(mirror_button, 6, 2, Qt::AlignLeft);
    mirror_label->setBuddy(mirror_button);

    QLabel *const other_label = new QLabel("other");
    layout->addWidget(other_label, 1, 6, Qt::AlignRight);
    KColorButton *const other_button = new KColorButton();
    other_button->setObjectName("kcfg_other");
    layout->addWidget(other_button, 1, 7, Qt::AlignLeft);
    other_label->setBuddy(other_button);

    QLabel *const pvmove_label = new QLabel("pvmove");
    layout->addWidget(pvmove_label, 2, 6, Qt::AlignRight);
    KColorButton *const pvmove_button = new KColorButton();
    pvmove_button->setObjectName("kcfg_pvmove");
    layout->addWidget(pvmove_button, 2, 7, Qt::AlignLeft);
    pvmove_label->setBuddy(pvmove_button);

    QLabel *const inactive_label = new QLabel("inactive");
    layout->addWidget(inactive_label, 3, 6,  Qt::AlignRight);
    KColorButton *const inactive_button = new KColorButton();
    inactive_button->setObjectName("kcfg_inactive");
    layout->addWidget(inactive_button, 3, 7, Qt::AlignLeft);
    inactive_label->setBuddy(inactive_button);

    QLabel *const thinvol_label = new QLabel("thin volume");
    layout->addWidget(thinvol_label, 4, 6, Qt::AlignRight);
    KColorButton *const thinvol_button = new KColorButton();
    thinvol_button->setObjectName("kcfg_thinvol");
    layout->addWidget(thinvol_button, 4, 7, Qt::AlignLeft);
    thinvol_label->setBuddy(thinvol_button);

    QLabel *const thinsnap_label = new QLabel("thin snapshot");
    layout->addWidget(thinsnap_label, 5, 6, Qt::AlignRight);
    KColorButton *const thinsnap_button = new KColorButton();
    thinsnap_button->setObjectName("kcfg_thinsnap");
    layout->addWidget(thinsnap_button, 5, 7, Qt::AlignLeft);
    thinsnap_label->setBuddy(thinsnap_button);

    return type;
}

QWidget *KvpmConfigDialog::otherColors()
{
    QWidget *const type = new QWidget;

    static QColor free_color;

    m_skeleton->setCurrentGroup("VolumeTypeColors");
    m_skeleton->addItemColor("free",     free_color,     Qt::green);

    static QColor primary_color;
    static QColor logical_color;
    static QColor extended_color;

    m_skeleton->setCurrentGroup("PartitionTypeColors");
    m_skeleton->addItemColor("primary",   primary_color,  Qt::cyan);
    m_skeleton->addItemColor("logical",   logical_color,  Qt::blue);
    m_skeleton->addItemColor("extended",  extended_color, Qt::darkGreen);

    QGridLayout *const layout = new QGridLayout();
    type->setLayout(layout);

    QLabel *const free_label = new QLabel("unused space");
    layout->addWidget(free_label, 1, 1, Qt::AlignRight);
    KColorButton *const free_button = new KColorButton();
    free_button->setToolTip(i18n("Any unused space in a volume group or unpartitioned space on a device"));
    free_button->setObjectName("kcfg_free");
    layout->addWidget(free_button, 1, 2, Qt::AlignLeft);
    free_label->setBuddy(free_button);

    QLabel *const extended_label = new QLabel("extended space");
    layout->addWidget(extended_label, 2, 1, Qt::AlignRight);
    KColorButton *const extended_button = new KColorButton();
    extended_button->setObjectName("kcfg_extended");
    extended_button->setToolTip(i18n("Any unpartitioned space inside an extended partition"));
    layout->addWidget(extended_button, 2, 2, Qt::AlignLeft);
    extended_label->setBuddy(extended_button);

    m_primary_label = new QLabel("primary partition");
    layout->addWidget(m_primary_label, 3, 1, Qt::AlignRight);
    m_primary_button = new KColorButton();
    m_primary_button->setObjectName("kcfg_primary");
    m_primary_button->setToolTip(i18n("An ordinary partition"));
    layout->addWidget(m_primary_button, 3, 2, Qt::AlignLeft);
    m_primary_label->setBuddy(m_primary_button);
    
    m_logical_label = new QLabel("logical partition");
    layout->addWidget(m_logical_label, 4, 1, Qt::AlignRight);
    m_logical_button = new KColorButton();
    m_logical_button->setObjectName("kcfg_logical");
    m_logical_button->setToolTip(i18n("A partition inside an extended partition"));
    layout->addWidget(m_logical_button, 4, 2, Qt::AlignLeft);
    m_logical_label->setBuddy(m_logical_button);
    
    layout->setRowMinimumHeight(1, 40);
    layout->setRowMinimumHeight(2, 40);
    layout->setRowMinimumHeight(3, 40);
    layout->setRowMinimumHeight(4, 40);
    layout->setRowStretch(5, 1);

    return type;
}

void KvpmConfigDialog::showOtherButtons(int index)
{
    if (index == 1) {
        m_primary_button->hide();
        m_logical_button->hide();
        m_primary_label->hide();
        m_logical_label->hide();
    } else {
        m_primary_button->show();
        m_logical_button->show();
        m_primary_label->show();
        m_logical_label->show();
    }
}

