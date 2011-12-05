/*
 *
 * 
 * Copyright (C) 2009, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
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

#include <KConfigSkeleton>
#include <KIcon>
#include <KIconLoader>
#include <KLocale>
#include <KEditListBox>
#include <KTabWidget>
#include <KListWidget>
#include <KColorButton>
#include <KSeparator>

#include <QtGui>

#include "executablefinder.h"



KvpmConfigDialog::KvpmConfigDialog( QWidget *parent, 
                                    const QString name, 
                                    KConfigSkeleton *const skeleton, 
                                    ExecutableFinder *const executableFinder ) 
    : KConfigDialog(parent, name, skeleton), 
      m_skeleton(skeleton),
      m_executable_finder(executableFinder) 
{
    setFaceType(KPageDialog::Auto);

    KPageWidgetItem *item;

    item = addPage(generalPage(), i18nc("The standard common options", "General") );
    item->setIcon( KIcon("configure") );
    item = addPage(colorsPage(), i18n("Colors") ); 
    item->setIcon( KIcon("color-picker") );
    item = addPage(programsPage(), i18n("Programs") ); 
    item->setIcon( KIcon("applications-system") );
}

KvpmConfigDialog::~KvpmConfigDialog()
{
    return;
}

QWidget *KvpmConfigDialog::generalPage()
{
    QWidget *const general = new QWidget;
    QVBoxLayout *const general_layout = new QVBoxLayout();
    QLabel *const banner = new QLabel( i18n("<b>Set columns to show in tables and tree views</b>") );
    banner->setAlignment(Qt::AlignCenter);
    general_layout->addWidget(banner);
    QHBoxLayout *const horizontal_layout = new QHBoxLayout();
    general_layout->addLayout(horizontal_layout);
    general->setLayout(general_layout);

    horizontal_layout->addWidget( deviceGroup() );
    horizontal_layout->addWidget( logicalGroup() );
    horizontal_layout->addWidget( physicalGroup() );
    horizontal_layout->addWidget( allGroup() );

    return general;
}

QWidget *KvpmConfigDialog::colorsPage()
{
    m_skeleton->setCurrentGroup("FilesystemColors");
    m_skeleton->addItemColor("ext2",   m_ext2_color);
    m_skeleton->addItemColor("ext3",   m_ext3_color);
    m_skeleton->addItemColor("ext4",   m_ext4_color);
    m_skeleton->addItemColor("btrfs",   m_btrfs_color);
    m_skeleton->addItemColor("reiser",  m_reiser_color);
    m_skeleton->addItemColor("reiser4", m_reiser4_color);
    m_skeleton->addItemColor("msdos", m_msdos_color);
    m_skeleton->addItemColor("jfs",   m_jfs_color);
    m_skeleton->addItemColor("xfs",   m_xfs_color);
    m_skeleton->addItemColor("hfs",   m_hfs_color);
    m_skeleton->addItemColor("ntfs",  m_ntfs_color);
    m_skeleton->addItemColor("none",  m_none_color);
    m_skeleton->addItemColor("free",  m_free_color);
    m_skeleton->addItemColor("swap",  m_swap_color);
    m_skeleton->addItemColor("physvol",  m_physical_color);

    QWidget *const colors = new QWidget;
    QVBoxLayout *const colors_layout = new QVBoxLayout();

    QGroupBox *const selection_box = new QGroupBox( i18n("Volume and Partition Colors") );
    QGridLayout *const selection_layout = new QGridLayout();

    QLabel *const message_label = new QLabel( i18n("These are the colors used to show the filesystem type \n"
                                                   "on a partition or volume in any graphical display. Colors\n"
                                                   "may also be selected for swap and unpartitioned space.") );

    KSeparator *const left_separator  = new KSeparator(Qt::Vertical);
    KSeparator *const right_separator = new KSeparator(Qt::Vertical);

    selection_layout->addWidget(left_separator,  1, 4, 5, 1);
    selection_layout->addWidget(right_separator, 1, 9, 5, 1);

    selection_layout->addWidget(message_label, 0, 0, 1, -1, Qt::AlignHCenter);

    QLabel *ext2_label = new QLabel("ext2");
    selection_layout->addWidget(ext2_label, 1, 1, Qt::AlignRight);
    m_ext2_button = new KColorButton( m_ext2_color );
    selection_layout->addWidget(m_ext2_button, 1, 2, Qt::AlignLeft);

    QLabel *ext3_label = new QLabel("ext3");
    selection_layout->addWidget(ext3_label, 2, 1, Qt::AlignRight);
    m_ext3_button = new KColorButton( m_ext3_color );
    selection_layout->addWidget(m_ext3_button, 2, 2, Qt::AlignLeft);

    QLabel *ext4_label = new QLabel("ext4");
    selection_layout->addWidget(ext4_label, 3, 1, Qt::AlignRight);
    m_ext4_button = new KColorButton( m_ext4_color );
    selection_layout->addWidget(m_ext4_button, 3, 2, Qt::AlignLeft);

    QLabel *btrfs_label = new QLabel("btrfs");
    selection_layout->addWidget(btrfs_label, 4, 1, Qt::AlignRight);
    m_btrfs_button = new KColorButton( m_btrfs_color );
    selection_layout->addWidget(m_btrfs_button, 4, 2, Qt::AlignLeft);

    QLabel *reiser_label = new QLabel("reiser");
    selection_layout->addWidget(reiser_label, 1, 6, Qt::AlignRight);
    m_reiser_button = new KColorButton( m_reiser_color );
    selection_layout->addWidget(m_reiser_button, 1, 7, Qt::AlignLeft);

    QLabel *reiser4_label = new QLabel("reiser4");
    selection_layout->addWidget(reiser4_label, 2, 6, Qt::AlignRight);
    m_reiser4_button = new KColorButton( m_reiser4_color );
    selection_layout->addWidget(m_reiser4_button, 2, 7, Qt::AlignLeft);

    QLabel *msdos_label = new QLabel("ms-dos");
    selection_layout->addWidget(msdos_label, 3, 6, Qt::AlignRight);
    m_msdos_button = new KColorButton( m_msdos_color );
    selection_layout->addWidget(m_msdos_button, 3, 7, Qt::AlignLeft);

    QLabel *jfs_label = new QLabel("jfs");
    selection_layout->addWidget(jfs_label, 1, 11, Qt::AlignRight);
    m_jfs_button = new KColorButton( m_jfs_color );
    selection_layout->addWidget(m_jfs_button, 1, 12, Qt::AlignLeft);

    QLabel *xfs_label = new QLabel("xfs");
    selection_layout->addWidget(xfs_label, 2, 11, Qt::AlignRight);
    m_xfs_button = new KColorButton( m_xfs_color );
    selection_layout->addWidget(m_xfs_button, 2, 12, Qt::AlignLeft);

    QLabel *swap_label = new QLabel("linux \nswap");
    selection_layout->addWidget(swap_label, 3, 11, Qt::AlignRight);
    m_swap_button = new KColorButton( m_swap_color );
    selection_layout->addWidget(m_swap_button, 3, 12, Qt::AlignLeft);

    QLabel *none_label = new QLabel("none");
    selection_layout->addWidget(none_label, 4, 6,  Qt::AlignRight);
    m_none_button = new KColorButton( m_none_color );
    selection_layout->addWidget(m_none_button, 4, 7, Qt::AlignLeft);

    QLabel *free_label = new QLabel("free \nspace");
    selection_layout->addWidget(free_label, 5, 6, Qt::AlignRight);
    m_free_button = new KColorButton( m_free_color );
    selection_layout->addWidget(m_free_button, 5, 7, Qt::AlignLeft);

    QLabel *hfs_label = new QLabel("hfs");
    selection_layout->addWidget(hfs_label, 4, 11,  Qt::AlignRight);
    m_hfs_button = new KColorButton( m_hfs_color );
    selection_layout->addWidget(m_hfs_button, 4, 12, Qt::AlignLeft);

    QLabel *ntfs_label = new QLabel("ntfs");
    selection_layout->addWidget(ntfs_label, 5, 11,  Qt::AlignRight);
    m_ntfs_button = new KColorButton( m_ntfs_color );
    selection_layout->addWidget(m_ntfs_button, 5, 12, Qt::AlignLeft);

    QLabel *physical_label = new QLabel("physical \nvolumes");
    selection_layout->addWidget(physical_label, 5, 1,  Qt::AlignRight);
    m_physical_button = new KColorButton( m_physical_color );
    selection_layout->addWidget(m_physical_button, 5, 2, Qt::AlignLeft);

    for(int row = 1; row < selection_layout->rowCount(); row++)
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

QWidget *KvpmConfigDialog::programsPage()
{
    m_executables_table = new QTableWidget();

    KTabWidget  *const programs = new KTabWidget;
    QVBoxLayout *const programs1_layout = new QVBoxLayout();
    QVBoxLayout *const programs2_layout = new QVBoxLayout();
    QWidget *const programs1 = new QWidget();
    QWidget *const programs2 = new QWidget();
    programs1->setLayout(programs1_layout);
    programs2->setLayout(programs2_layout);

    m_skeleton->setCurrentGroup("SystemPaths");
    m_skeleton->addItemStringList("SearchPath", m_search_entries, QStringList());

    m_edit_list = new KEditListBox();
    programs1_layout->addWidget(m_edit_list);

    programs->insertTab( 1, programs1, "Search path");
    m_edit_list->insertStringList( m_search_entries );

    programs2_layout->addWidget(m_executables_table);
    programs->insertTab( 1, programs2, "Executables");

    fillExecutablesTable();

    return programs;
} 

void KvpmConfigDialog::updateSettings()
{
    m_search_entries = m_edit_list->items();

    for( int x = 0; x < m_search_entries.size(); x++)
      if( ! m_search_entries[x].endsWith('/') )
	m_search_entries[x].append('/');

    m_ext2_color  = m_ext2_button->color();
    m_ext3_color  = m_ext3_button->color();
    m_ext4_color  = m_ext4_button->color();
    m_btrfs_color = m_btrfs_button->color();
    m_ntfs_color  = m_ntfs_button->color();
    m_xfs_color   = m_xfs_button->color();
    m_jfs_color   = m_jfs_button->color();
    m_hfs_color   = m_hfs_button->color();
    m_swap_color  = m_swap_button->color();
    m_msdos_color = m_msdos_button->color();
    m_none_color  = m_none_button->color();
    m_free_color  = m_free_button->color();
    m_reiser_color   = m_reiser_button->color();
    m_reiser4_color  = m_reiser4_button->color();
    m_physical_color = m_physical_button->color();

    m_device_column    = m_device_check->isChecked();
    m_partition_column = m_partition_check->isChecked();
    m_capacity_column  = m_capacity_check->isChecked();
    m_devremaining_column  = m_devremaining_check->isChecked();
    m_usage_column     = m_usage_check->isChecked();
    m_group_column     = m_group_check->isChecked();
    m_flags_column     = m_flags_check->isChecked();
    m_mount_column     = m_mount_check->isChecked();
    m_expand_parts     = m_expandparts_check->isChecked();

    m_volume_column      = m_volume_check->isChecked();
    m_size_column        = m_size_check->isChecked();
    m_remaining_column   = m_remaining_check->isChecked();
    m_type_column        = m_type_check->isChecked();
    m_filesystem_column  = m_filesystem_check->isChecked();
    m_stripes_column     = m_stripes_check->isChecked();
    m_stripesize_column  = m_stripesize_check->isChecked();
    m_snapmove_column    = m_snapmove_check->isChecked();
    m_state_column       = m_state_check->isChecked();
    m_access_column      = m_access_check->isChecked();
    m_tags_column        = m_tags_check->isChecked();
    m_mountpoints_column = m_mountpoints_check->isChecked();

    m_pvname_column  = m_pvname_check->isChecked();
    m_pvsize_column  = m_pvsize_check->isChecked();
    m_pvremaining_column  = m_pvremaining_check->isChecked();
    m_pvused_column  = m_pvused_check->isChecked();
    m_pvstate_column = m_pvstate_check->isChecked();
    m_pvallocate_column = m_pvallocate_check->isChecked();
    m_pvtags_column     = m_pvtags_check->isChecked();
    m_pvlvnames_column  = m_pvlvnames_check->isChecked();

    if( m_percent_radio->isChecked() ){
        m_show_percent = true;
        m_show_total   = false;
    }
    else if( m_total_radio->isChecked() ){
        m_show_percent = false;
        m_show_total   = true;
    }
    else{
        m_show_percent = true;
        m_show_total   = true;
    }

    m_fs_warn_percent = m_fs_warn_spin->value();
    m_pv_warn_percent = m_pv_warn_spin->value();

    m_skeleton->writeConfig();

    m_executable_finder->reload();
    fillExecutablesTable();
}

void KvpmConfigDialog::updateWidgetsDefault()
{
    QStringList default_entries;

    default_entries << "/sbin/" 
		    << "/usr/sbin/" 
		    << "/bin/" 
		    << "/usr/bin/" 
		    << "/usr/local/bin/"
		    << "/usr/local/sbin/";

    m_edit_list->clear();
    m_edit_list->insertStringList( default_entries );

    m_ext2_button->setColor(Qt::blue);
    m_ext3_button->setColor(Qt::darkBlue);
    m_ext4_button->setColor(Qt::cyan);
    m_btrfs_button->setColor(Qt::yellow);
    m_swap_button->setColor(Qt::lightGray);
    m_none_button->setColor(Qt::black);
    m_free_button->setColor(Qt::green);
    m_xfs_button->setColor(Qt::darkCyan);
    m_hfs_button->setColor(Qt::darkMagenta);
    m_ntfs_button->setColor(Qt::darkGray);
    m_jfs_button->setColor(Qt::magenta);
    m_msdos_button->setColor(Qt::darkYellow);
    m_reiser_button->setColor(Qt::red);
    m_reiser4_button->setColor(Qt::darkRed);
    m_physical_button->setColor(Qt::darkGreen);

    m_device_check->setChecked(true);
    m_partition_check->setChecked(true);
    m_capacity_check->setChecked(true);
    m_devremaining_check->setChecked(true);
    m_usage_check->setChecked(true);
    m_group_check->setChecked(true);
    m_flags_check->setChecked(true);
    m_mount_check->setChecked(true);
    m_expandparts_check->setChecked(true);

    m_volume_check->setChecked(true);
    m_size_check->setChecked(true);
    m_remaining_check->setChecked(true);
    m_type_check->setChecked(true);
    m_filesystem_check->setChecked(false);
    m_stripes_check->setChecked(false);
    m_stripesize_check->setChecked(false);
    m_snapmove_check->setChecked(true);
    m_state_check->setChecked(true);
    m_access_check->setChecked(false);
    m_tags_check->setChecked(true);
    m_mountpoints_check->setChecked(false);

    m_pvname_check->setChecked(true);
    m_pvsize_check->setChecked(true);
    m_pvremaining_check->setChecked(true);
    m_pvused_check->setChecked(false);
    m_pvstate_check->setChecked(false);
    m_pvallocate_check->setChecked(true);
    m_pvtags_check->setChecked(true);
    m_pvlvnames_check->setChecked(true);

    m_both_radio->setChecked(true);
    m_fs_warn_spin->setValue(10);
    m_pv_warn_spin->setValue(0);
}

void KvpmConfigDialog::fillExecutablesTable()
{
    QTableWidgetItem *table_widget_item = NULL;

    const QStringList all_names = m_executable_finder->getAllNames();
    const QStringList all_paths = m_executable_finder->getAllPaths();
    const QStringList not_found = m_executable_finder->getNotFound();

    m_executables_table->clear();
    m_executables_table->setColumnCount(2);
    m_executables_table->setRowCount( all_names.size() + not_found.size() );

    for(int x = 0; x < not_found.size(); x++){
        table_widget_item = new QTableWidgetItem( not_found[x] );
	m_executables_table->setItem(x, 0, table_widget_item);

	table_widget_item = new QTableWidgetItem( KIcon("dialog-error"), "Not Found" );
	m_executables_table->setItem(x, 1, table_widget_item);
    }

    // these lists should be the same length, but just in case...

    for(int x = 0; (x < all_names.size()) && (x < all_paths.size()); x++){

        table_widget_item = new QTableWidgetItem( all_names[x] );
	m_executables_table->setItem(x + not_found.size(), 0, table_widget_item);

	table_widget_item = new QTableWidgetItem( all_paths[x] );
	m_executables_table->setItem(x + not_found.size(), 1, table_widget_item);

    }
    m_executables_table->resizeColumnsToContents();
    m_executables_table->setAlternatingRowColors( true );
    m_executables_table->verticalHeader()->hide();
}

bool KvpmConfigDialog::isDefault()
{
    return false;    // This keeps the "defaults" button enabled
}

bool KvpmConfigDialog::hasChanged()
{
    return true;     // This keeps the "apply" button enabled
}

QGroupBox *KvpmConfigDialog::deviceGroup()
{
    QGroupBox *const device_group = new QGroupBox( i18n("Device tree") );
    QVBoxLayout *const device_layout = new QVBoxLayout();
    device_group->setLayout(device_layout);

    m_skeleton->setCurrentGroup("DeviceTreeColumns");
    m_skeleton->addItemBool( "device",      m_device_column );
    m_skeleton->addItemBool( "partition",   m_partition_column );
    m_skeleton->addItemBool( "capacity",    m_capacity_column );
    m_skeleton->addItemBool( "remaining",   m_devremaining_column );
    m_skeleton->addItemBool( "usage",       m_usage_column );
    m_skeleton->addItemBool( "group",       m_group_column );
    m_skeleton->addItemBool( "flags",       m_flags_column );
    m_skeleton->addItemBool( "mount",       m_mount_column );
    m_skeleton->addItemBool( "expandparts", m_expand_parts );

    m_device_check      = new QCheckBox( i18n("Device name") );
    m_partition_check   = new QCheckBox( i18n("Partition type") );
    m_capacity_check    = new QCheckBox( i18n("Capacity") );
    m_devremaining_check = new QCheckBox( i18n("Remaining space") );
    m_usage_check       = new QCheckBox( i18n("Usage of device") );
    m_group_check       = new QCheckBox( i18n("Volume group") );
    m_flags_check       = new QCheckBox( i18n("Partition flags") );
    m_mount_check       = new QCheckBox( i18n("Mount point") );
    m_expandparts_check = new QCheckBox( i18n("Expand device tree") );

    m_device_check->setToolTip( i18n("Show the path to the device, /dev/sda1 for example.") );
    m_partition_check->setToolTip( i18n("Show the type of partition, 'extended' for example.") );
    m_capacity_check->setToolTip( i18n("Show the storage capacity in megabytes, gigabytes or terabytes.") );
    m_devremaining_check->setToolTip( i18n("Show the remaining storage in megabytes, gigabytes or terabytes.") );
    m_usage_check->setToolTip( i18n("Show how the partition is being used. Usually the type of filesystem, such as ext4, \n"
                                    "swap space or as a physcial volume.") );
    m_group_check->setToolTip( i18n("If the partition is a physical volume this column shows the volume group it is in.") );
    m_flags_check->setToolTip( i18n("Show any flags, such a 'boot.'") );
    m_mount_check->setToolTip( i18n("Show the mount point if the partition has a mounted filesystem.") );
    m_expandparts_check->setToolTip( i18n("This determines if all partitions of all devices get shown at start up. \n "
                                          "The user can still expand or collapse the items by clicking on them.") );
    m_device_check->setChecked(m_device_column);
    m_partition_check->setChecked(m_partition_column);
    m_capacity_check->setChecked(m_capacity_column);
    m_devremaining_check->setChecked(m_devremaining_column);
    m_usage_check->setChecked(m_usage_column);
    m_group_check->setChecked(m_group_column);
    m_flags_check->setChecked(m_flags_column);
    m_mount_check->setChecked(m_mount_column);
    m_expandparts_check->setChecked(m_expand_parts);

    device_layout->addWidget(m_device_check);
    device_layout->addWidget(m_partition_check);
    device_layout->addWidget(m_capacity_check);
    device_layout->addWidget(m_devremaining_check);
    device_layout->addWidget(m_usage_check);
    device_layout->addWidget(m_group_check);
    device_layout->addWidget(m_flags_check);
    device_layout->addWidget(m_mount_check);
    device_layout->addWidget(new KSeparator(Qt::Horizontal));
    QLabel *const label = new QLabel( i18n("At start up:") );
    label->setAlignment(Qt::AlignCenter);
    device_layout->addWidget(label);
    device_layout->addWidget(m_expandparts_check);
    device_layout->addStretch();

    return device_group;
}

QGroupBox *KvpmConfigDialog::physicalGroup()
{
    QGroupBox *const physical_group = new QGroupBox( i18n("Physical volume table") );
    QVBoxLayout *const physical_layout = new QVBoxLayout();
    physical_group->setLayout(physical_layout);

    m_skeleton->setCurrentGroup("PhysicalTreeColumns");

    m_skeleton->addItemBool( "pvname",      m_pvname_column );
    m_skeleton->addItemBool( "pvsize",      m_pvsize_column );
    m_skeleton->addItemBool( "pvremaining", m_pvremaining_column );
    m_skeleton->addItemBool( "pvused",      m_pvused_column );
    m_skeleton->addItemBool( "pvstate",     m_pvstate_column );
    m_skeleton->addItemBool( "pvallocate",  m_pvallocate_column );
    m_skeleton->addItemBool( "pvtags",      m_pvtags_column );
    m_skeleton->addItemBool( "pvlvnames",   m_pvlvnames_column );

    m_pvname_check  = new QCheckBox( i18n("Volume name") );
    m_pvsize_check  = new QCheckBox( i18n("Size") );
    m_pvremaining_check  = new QCheckBox( i18n("Remaining space") );
    m_pvused_check  = new QCheckBox( i18n("Used space") );
    m_pvstate_check = new QCheckBox( i18n("State") );
    m_pvallocate_check = new QCheckBox( i18n("Allocatable") );
    m_pvtags_check     = new QCheckBox( i18n("Tags") );
    m_pvlvnames_check  = new QCheckBox( i18n("Logical Volumes") );

    m_pvname_check->setToolTip( i18n("Show the path to the device, /dev/sda1 for example.") );
    m_pvsize_check->setToolTip( i18n("Show the storage capacity in megabytes, gigabytes or terabytes.") );
    m_pvremaining_check->setToolTip( i18n("Show the remaining storage in megabytes, gigabytes or terabytes.") );
    m_pvused_check->setToolTip( i18n("Show the used storage in megabytes, gigabytes or terabytes.") );
    m_pvstate_check->setToolTip( i18n("Show the state, either 'active' or 'inactive.'") );
    m_pvallocate_check->setToolTip( i18n("Shows whether or not the physical volume can have its extents allocated.") );
    m_pvtags_check->setToolTip( i18n("List any tags associated with the physical volume.") );
    m_pvlvnames_check->setToolTip( i18n("List any logical volumes associated with the physical volume.") );

    m_pvname_check->setChecked(m_pvname_column);
    m_pvsize_check->setChecked(m_pvsize_column);
    m_pvremaining_check->setChecked(m_pvremaining_column);
    m_pvused_check->setChecked(m_pvused_column);
    m_pvstate_check->setChecked(m_pvstate_column);
    m_pvallocate_check->setChecked(m_pvallocate_column);
    m_pvtags_check->setChecked(m_pvtags_column);
    m_pvlvnames_check->setChecked(m_pvlvnames_column);

    physical_layout->addWidget(m_pvname_check);
    physical_layout->addWidget(m_pvsize_check);
    physical_layout->addWidget(m_pvremaining_check);
    physical_layout->addWidget(m_pvused_check);
    physical_layout->addWidget(m_pvstate_check);
    physical_layout->addWidget(m_pvallocate_check);
    physical_layout->addWidget(m_pvtags_check);
    physical_layout->addWidget(m_pvlvnames_check);
    physical_layout->addStretch();

    return physical_group;
}

QGroupBox *KvpmConfigDialog::logicalGroup()
{
    QGroupBox *const logical_group = new QGroupBox( i18n("Logical volume tree") );
    QVBoxLayout *const logical_layout = new QVBoxLayout();
    logical_group->setLayout(logical_layout);

    m_skeleton->setCurrentGroup("VolumeTreeColumns");
    m_skeleton->addItemBool( "volume",      m_volume_column );
    m_skeleton->addItemBool( "size",        m_size_column );
    m_skeleton->addItemBool( "remaining",   m_remaining_column );
    m_skeleton->addItemBool( "type",        m_type_column );
    m_skeleton->addItemBool( "filesystem",  m_filesystem_column );
    m_skeleton->addItemBool( "stripes",     m_stripes_column );
    m_skeleton->addItemBool( "stripesize",  m_stripesize_column );
    m_skeleton->addItemBool( "snapmove",    m_snapmove_column );
    m_skeleton->addItemBool( "state",       m_state_column );
    m_skeleton->addItemBool( "access",      m_access_column );
    m_skeleton->addItemBool( "tags",        m_tags_column );
    m_skeleton->addItemBool( "mountpoints", m_mountpoints_column );

    m_volume_check      = new QCheckBox( i18n("Volume name") );
    m_size_check        = new QCheckBox( i18n("Size") );
    m_remaining_check   = new QCheckBox( i18n("Remaining space") );
    m_type_check        = new QCheckBox( i18n("Volume type") );
    m_filesystem_check  = new QCheckBox( i18n("Filesystem type") );
    m_stripes_check     = new QCheckBox( i18n("Stripe count") );
    m_stripesize_check  = new QCheckBox( i18n("Stripe size") );
    m_snapmove_check    = new QCheckBox( i18n("(\%)Snap/Copy") );
    m_state_check       = new QCheckBox( i18n("Volume state") );
    m_access_check      = new QCheckBox( i18n("Volume access") );
    m_tags_check        = new QCheckBox( i18n("Tags") );
    m_mountpoints_check = new QCheckBox( i18n("Mount point") );

    m_volume_check->setToolTip( i18n("Show the volume name.") );
    m_size_check->setToolTip( i18n("Show the storage capacity in megabytes, gigabytes or terabytes.") );
    m_remaining_check->setToolTip( i18n("Show the remaining storage in megabytes, gigabytes or terabytes.") );
    m_type_check->setToolTip( i18n("Show the type of volume, 'mirror' or 'linear,' for example.") );
    m_filesystem_check->setToolTip( i18n("Show the filesystem type on the volume, 'ext3' or 'swap,' for example.") );
    m_stripes_check->setToolTip( i18n("Show the number of stripes on the volume, if it is striped") );
    m_stripesize_check->setToolTip( i18n("Show the size of the stripes, if it is striped") );
    m_snapmove_check->setToolTip( i18n("For a mirror, show the percentage of the mirror synced. \n"
                                       "For a snapshot, show the percentage of the snapshot used. \n"
                                       "For a pvmove, show the percentage of the move completed.") );
    m_state_check->setToolTip( i18n("Show the state, 'active' or 'invalid,' for example.") );
    m_access_check->setToolTip( i18n("Show access, either read only or read and write.") );
    m_tags_check->setToolTip( i18n("List any tags associated with the volume") );
    m_mountpoints_check->setToolTip( i18n("Show the mount point if the partition has a mounted filesystem.") );

    m_volume_check->setChecked(m_volume_column);
    m_size_check->setChecked(m_size_column);
    m_remaining_check->setChecked(m_remaining_column);
    m_type_check->setChecked(m_type_column);
    m_filesystem_check->setChecked(m_filesystem_column);
    m_stripes_check->setChecked(m_stripes_column);
    m_stripesize_check->setChecked(m_stripesize_column);
    m_snapmove_check->setChecked(m_snapmove_column);
    m_state_check->setChecked(m_state_column);
    m_access_check->setChecked(m_access_column);
    m_tags_check->setChecked(m_tags_column);
    m_mountpoints_check->setChecked(m_mountpoints_column);

    logical_layout->addWidget(m_volume_check);
    logical_layout->addWidget(m_size_check);
    logical_layout->addWidget(m_remaining_check);
    logical_layout->addWidget(m_type_check);
    logical_layout->addWidget(m_filesystem_check);
    logical_layout->addWidget(m_stripes_check);
    logical_layout->addWidget(m_stripesize_check);
    logical_layout->addWidget(m_snapmove_check);
    logical_layout->addWidget(m_state_check);
    logical_layout->addWidget(m_access_check);
    logical_layout->addWidget(m_tags_check);
    logical_layout->addWidget(m_mountpoints_check);

    return logical_group;
}

QGroupBox *KvpmConfigDialog::allGroup()
{
    QGroupBox *const all_group = new QGroupBox( i18n("All trees and tables") );
    QGroupBox *const percent_group = new QGroupBox( i18n("Remaining and used space") );

    QVBoxLayout *const all_layout = new QVBoxLayout();
    QVBoxLayout *const percent_layout = new QVBoxLayout();
    all_group->setLayout(all_layout);
    percent_group->setLayout(percent_layout);

    m_skeleton->setCurrentGroup("AllTreeColumns");
    m_skeleton->addItemBool("percent", m_show_percent);
    m_skeleton->addItemBool("total",   m_show_total);
    m_skeleton->addItemInt("fs_warn",  m_fs_warn_percent);
    m_skeleton->addItemInt("pv_warn",  m_pv_warn_percent);

    m_percent_radio = new QRadioButton( i18n("Show percentage") );
    m_total_radio   = new QRadioButton( i18n("Show total") );
    m_both_radio    = new QRadioButton( i18n("Show both") );

    if(m_show_percent && !m_show_total)
        m_percent_radio->setChecked(true);
    else if(!m_show_percent && m_show_total)
        m_total_radio->setChecked(true);
    else
        m_both_radio->setChecked(true);

    percent_layout->addWidget(m_total_radio);
    percent_layout->addWidget(m_percent_radio);
    percent_layout->addWidget(m_both_radio);
    percent_layout->addWidget(new KSeparator(Qt::Horizontal));

    QHBoxLayout *const warn_layout = new QHBoxLayout;
    QHBoxLayout *const fs_warn_layout = new QHBoxLayout;
    QHBoxLayout *const pv_warn_layout = new QHBoxLayout;
    warn_layout->addWidget( new QLabel( i18n("Show warning icon") ) );
    QLabel *const warn_icon = new QLabel;
    warn_icon->setPixmap( KIcon("exclamation").pixmap(16, 16) );
    warn_layout->addWidget(warn_icon);
    percent_layout->addLayout(warn_layout);
    percent_layout->addWidget( new QLabel( i18n("when space falls to or below:") ) );

    m_fs_warn_spin = new QSpinBox;
    m_fs_warn_spin->setRange(0, 99);
    m_fs_warn_spin->setSingleStep(1);
    m_fs_warn_spin->setPrefix("% ");
    m_fs_warn_spin->setSpecialValueText( i18n("Never") );
    m_fs_warn_spin->setValue(m_fs_warn_percent);
    fs_warn_layout->addWidget(m_fs_warn_spin);
    fs_warn_layout->addWidget( new QLabel( i18n("on a filesystem") ) );
    fs_warn_layout->addStretch();

    m_pv_warn_spin = new QSpinBox;
    m_pv_warn_spin->setRange(0, 99);
    m_pv_warn_spin->setSingleStep(1);
    m_pv_warn_spin->setPrefix("% ");
    m_pv_warn_spin->setSpecialValueText( i18n("Never") );
    m_pv_warn_spin->setValue(m_pv_warn_percent);
    pv_warn_layout->addWidget(m_pv_warn_spin);
    pv_warn_layout->addWidget( new QLabel( i18n("on a physical volume") ) );
    pv_warn_layout->addStretch();

    percent_layout->addLayout(fs_warn_layout);
    percent_layout->addLayout(pv_warn_layout);

    all_layout->addWidget(percent_group);
    all_layout->addStretch();

    return all_group;
}

