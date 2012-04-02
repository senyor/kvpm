/*
 *
 *
 * Copyright (C) 2009, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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
#include <KIcon>
#include <KIconLoader>
#include <KIntSpinBox>
#include <KListWidget>
#include <KLocale>
#include <KMessageBox>
#include <KPageWidgetItem>
#include <KSeparator>
#include <KTabWidget>

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QRadioButton>
#include <QString>
#include <QTableWidget>
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
    setDefaultButton(KDialog::Cancel);

    addPage(generalPage(), i18nc("The standard common options", "General"), QString("configure"));
    addPage(colorsPage(), i18n("Colors"), QString("color-picker"));
    addPage(programsPage(), i18n("Programs"), QString("applications-system"));
}

KvpmConfigDialog::~KvpmConfigDialog()
{
    m_skeleton->deleteLater();
}

KTabWidget *KvpmConfigDialog::generalPage()
{
    KTabWidget *const tabwidget = new KTabWidget;
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

QWidget *KvpmConfigDialog::colorsPage()
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
    static QColor none_color;
    static QColor free_color;
    static QColor swap_color;
    static QColor physvol_color;

    m_skeleton->setCurrentGroup("FilesystemColors");
    m_skeleton->addItemColor("ext2",    ext2_color,    Qt::blue);
    m_skeleton->addItemColor("ext3",    ext3_color,    Qt::darkBlue);
    m_skeleton->addItemColor("ext4",    ext4_color,    Qt::cyan);
    m_skeleton->addItemColor("btrfs",   btrfs_color,   Qt::yellow);
    m_skeleton->addItemColor("reiser",  reiser_color,  Qt::red);
    m_skeleton->addItemColor("reiser4", reiser4_color, Qt::darkRed);
    m_skeleton->addItemColor("msdos",   msdos_color,   Qt::darkYellow);
    m_skeleton->addItemColor("jfs",     jfs_color,     Qt::magenta);
    m_skeleton->addItemColor("xfs",     xfs_color,     Qt::darkCyan);
    m_skeleton->addItemColor("hfs",     hfs_color,     Qt::darkMagenta);
    m_skeleton->addItemColor("ntfs",    ntfs_color,    Qt::darkGray);
    m_skeleton->addItemColor("none",    none_color,    Qt::black);
    m_skeleton->addItemColor("free",    free_color,    Qt::green);
    m_skeleton->addItemColor("swap",    swap_color,    Qt::lightGray);
    m_skeleton->addItemColor("physvol", physvol_color, Qt::darkGreen);

    QWidget *const colors = new QWidget;
    QVBoxLayout *const colors_layout = new QVBoxLayout();

    QGroupBox *const selection_box = new QGroupBox(i18n("Volume and Partition Colors"));
    QGridLayout *const selection_layout = new QGridLayout();

    QLabel *const message_label = new QLabel(i18n("These are the colors used to show the filesystem type \n"
            "on a partition or volume in any graphical display. Colors\n"
            "may also be selected for swap and unpartitioned space."));

    KSeparator *const left_separator  = new KSeparator(Qt::Vertical);
    KSeparator *const right_separator = new KSeparator(Qt::Vertical);

    selection_layout->addWidget(left_separator,  1, 4, 5, 1);
    selection_layout->addWidget(right_separator, 1, 9, 5, 1);
    selection_layout->addWidget(message_label, 0, 0, 1, -1, Qt::AlignHCenter);

    QLabel *const ext2_label = new QLabel("ext2");
    selection_layout->addWidget(ext2_label, 1, 1, Qt::AlignRight);
    KColorButton *const ext2_button = new KColorButton();
    ext2_button->setObjectName("kcfg_ext2");
    selection_layout->addWidget(ext2_button, 1, 2, Qt::AlignLeft);
    ext2_label->setBuddy(ext2_button);

    QLabel *const ext3_label = new QLabel("ext3");
    selection_layout->addWidget(ext3_label, 2, 1, Qt::AlignRight);
    KColorButton *const ext3_button = new KColorButton();
    ext3_button->setObjectName("kcfg_ext3");
    selection_layout->addWidget(ext3_button, 2, 2, Qt::AlignLeft);
    ext3_label->setBuddy(ext3_button);

    QLabel *const ext4_label = new QLabel("ext4");
    selection_layout->addWidget(ext4_label, 3, 1, Qt::AlignRight);
    KColorButton *const ext4_button = new KColorButton();
    ext4_button->setObjectName("kcfg_ext4");
    selection_layout->addWidget(ext4_button, 3, 2, Qt::AlignLeft);
    ext4_label->setBuddy(ext4_button);

    QLabel *const btrfs_label = new QLabel("btrfs");
    selection_layout->addWidget(btrfs_label, 4, 1, Qt::AlignRight);
    KColorButton *const btrfs_button = new KColorButton();
    btrfs_button->setObjectName("kcfg_btrfs");
    selection_layout->addWidget(btrfs_button, 4, 2, Qt::AlignLeft);
    btrfs_label->setBuddy(btrfs_button);

    QLabel *const reiser_label = new QLabel("reiser");
    selection_layout->addWidget(reiser_label, 1, 6, Qt::AlignRight);
    KColorButton *const reiser_button = new KColorButton();
    reiser_button->setObjectName("kcfg_reiser");
    selection_layout->addWidget(reiser_button, 1, 7, Qt::AlignLeft);
    reiser_label->setBuddy(reiser_button);

    QLabel *const reiser4_label = new QLabel("reiser4");
    selection_layout->addWidget(reiser4_label, 2, 6, Qt::AlignRight);
    KColorButton *const reiser4_button = new KColorButton();
    reiser4_button->setObjectName("kcfg_reiser4");
    selection_layout->addWidget(reiser4_button, 2, 7, Qt::AlignLeft);
    reiser4_label->setBuddy(reiser4_button);

    QLabel *const msdos_label = new QLabel("ms-dos");
    selection_layout->addWidget(msdos_label, 3, 6, Qt::AlignRight);
    KColorButton *const msdos_button = new KColorButton();
    msdos_button->setObjectName("kcfg_msdos");
    selection_layout->addWidget(msdos_button, 3, 7, Qt::AlignLeft);
    msdos_label->setBuddy(msdos_button);

    QLabel *const jfs_label = new QLabel("jfs");
    selection_layout->addWidget(jfs_label, 1, 11, Qt::AlignRight);
    KColorButton *const jfs_button = new KColorButton();
    jfs_button->setObjectName("kcfg_jfs");
    selection_layout->addWidget(jfs_button, 1, 12, Qt::AlignLeft);
    jfs_label->setBuddy(jfs_button);

    QLabel *const xfs_label = new QLabel("xfs");
    selection_layout->addWidget(xfs_label, 2, 11, Qt::AlignRight);
    KColorButton *const xfs_button = new KColorButton();
    xfs_button->setObjectName("kcfg_xfs");
    selection_layout->addWidget(xfs_button, 2, 12, Qt::AlignLeft);
    xfs_label->setBuddy(xfs_button);

    QLabel *const swap_label = new QLabel("linux \nswap");
    selection_layout->addWidget(swap_label, 3, 11, Qt::AlignRight);
    KColorButton *const swap_button = new KColorButton();
    swap_button->setObjectName("kcfg_swap");
    selection_layout->addWidget(swap_button, 3, 12, Qt::AlignLeft);
    swap_label->setBuddy(swap_button);

    QLabel *const none_label = new QLabel("none");
    selection_layout->addWidget(none_label, 4, 6,  Qt::AlignRight);
    KColorButton *const none_button = new KColorButton();
    none_button->setObjectName("kcfg_none");
    selection_layout->addWidget(none_button, 4, 7, Qt::AlignLeft);
    none_label->setBuddy(none_button);

    QLabel *const free_label = new QLabel("free \nspace");
    selection_layout->addWidget(free_label, 5, 6, Qt::AlignRight);
    KColorButton *const free_button = new KColorButton();
    free_button->setObjectName("kcfg_free");
    selection_layout->addWidget(free_button, 5, 7, Qt::AlignLeft);
    free_label->setBuddy(free_button);

    QLabel *const hfs_label = new QLabel("hfs");
    selection_layout->addWidget(hfs_label, 4, 11,  Qt::AlignRight);
    KColorButton *const hfs_button = new KColorButton();
    hfs_button->setObjectName("kcfg_hfs");
    selection_layout->addWidget(hfs_button, 4, 12, Qt::AlignLeft);
    hfs_label->setBuddy(hfs_button);

    QLabel *const ntfs_label = new QLabel("ntfs");
    selection_layout->addWidget(ntfs_label, 5, 11,  Qt::AlignRight);
    KColorButton *const ntfs_button = new KColorButton();
    ntfs_button->setObjectName("kcfg_ntfs");
    selection_layout->addWidget(ntfs_button, 5, 12, Qt::AlignLeft);
    ntfs_label->setBuddy(ntfs_button);

    QLabel *const physical_label = new QLabel("physical \nvolumes");
    selection_layout->addWidget(physical_label, 5, 1,  Qt::AlignRight);
    KColorButton *const physical_button = new KColorButton();
    physical_button->setObjectName("kcfg_physvol");
    selection_layout->addWidget(physical_button, 5, 2, Qt::AlignLeft);
    physical_label->setBuddy(physical_button);

    for (int row = 1; row < selection_layout->rowCount(); row++)
        selection_layout->setRowStretch(row, 1);

    selection_layout->setColumnStretch(0, 30);
    selection_layout->setColumnStretch(3, 30);
    selection_layout->setColumnStretch(5, 30);
    selection_layout->setColumnStretch(8, 30);
    selection_layout->setColumnStretch(10, 30);
    selection_layout->setColumnStretch(13, 30);

    selection_box->setLayout(selection_layout);
    colors_layout->addWidget(selection_box);
    colors->setLayout(colors_layout);

    return colors;
}

KTabWidget *KvpmConfigDialog::programsPage()
{
    m_executables_table = new QTableWidget();
    m_executables_table->setColumnCount(2);

    const QStringList headers = QStringList() << i18n("Program name") << i18n("Full path");

    m_executables_table->setHorizontalHeaderLabels(headers);

    KTabWidget  *const programs = new KTabWidget;
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
    QTableWidgetItem *table_item = NULL;

    const QStringList all_names = m_executable_finder->getAllNames();
    const QStringList all_paths = m_executable_finder->getAllPaths();
    const QStringList not_found = m_executable_finder->getNotFound();

    m_executables_table->clearContents();
    m_executables_table->setRowCount(all_names.size() + not_found.size());

    for (int x = 0; x < not_found.size(); x++) {
        table_item = new QTableWidgetItem(not_found[x]);
        m_executables_table->setItem(x, 0, table_item);

        table_item = new QTableWidgetItem(KIcon("dialog-error"), "Not Found");
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
    QCheckBox *const snapmove_check    = new QCheckBox(i18n("(\%)Snap/Copy"));
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
    warn_icon->setPixmap(KIcon("exclamation").pixmap(16, 16));
    warn_layout->addWidget(warn_icon);
    percent_layout->addLayout(warn_layout);
    percent_layout->addWidget(new QLabel(i18n("when space falls to or below:")));

    KIntSpinBox *const fs_warn_spin = new KIntSpinBox;
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

    KIntSpinBox *const pv_warn_spin = new KIntSpinBox;
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
