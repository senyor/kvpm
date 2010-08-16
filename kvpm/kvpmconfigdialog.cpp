/*
 *
 * 
 * Copyright (C) 2009, 2010 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <KConfigSkeleton>
#include <KIconLoader>
#include <KLocale>
#include <KEditListBox>
#include <KTabWidget>
#include <KListWidget>
#include <KColorButton>
#include <KSeparator>

#include <QtGui>
#include <QListWidgetItem>
#include <QTableWidget>

#include "kvpmconfigdialog.h"
#include "executablefinder.h"


extern ExecutableFinder *g_executable_finder;


bool config_kvpm()
{
  KConfigSkeleton *skeleton = new KConfigSkeleton();
  KvpmConfigDialog *dialog  = new KvpmConfigDialog( NULL, "settings", skeleton );

  dialog->exec();

  return false;
}
 

KvpmConfigDialog::KvpmConfigDialog( QWidget *parent, QString name, KConfigSkeleton *skeleton ) 
  : KConfigDialog(  parent, name, skeleton), m_skeleton(skeleton) 
{
    setFaceType(KPageDialog::List);

    buildGeneralPage();
    buildColorsPage();
    buildProgramsPage();
}

KvpmConfigDialog::~KvpmConfigDialog()
{
    delete m_kvpm_config;
    delete m_system_paths_group;
}

void KvpmConfigDialog::buildGeneralPage()
{
    QWidget *general = new QWidget;
    QHBoxLayout *general_layout = new QHBoxLayout();
    general->setLayout(general_layout);

    QGroupBox *device_columns_group = new QGroupBox( i18n("Show these columns in device tree") );
    QGroupBox *volume_columns_group = new QGroupBox( i18n("Show these columns in volume tree") );
    general_layout->addWidget(device_columns_group);
    general_layout->addWidget(volume_columns_group);
    QVBoxLayout *device_layout = new QVBoxLayout();
    QVBoxLayout *volume_layout = new QVBoxLayout();
    device_columns_group->setLayout(device_layout);
    volume_columns_group->setLayout(volume_layout);

    m_skeleton->setCurrentGroup("DeviceTreeColumns");
    m_skeleton->addItemBool( "device",    m_device_column );
    m_skeleton->addItemBool( "partition", m_partition_column );
    m_skeleton->addItemBool( "capacity",  m_capacity_column );
    m_skeleton->addItemBool( "used",      m_used_column );
    m_skeleton->addItemBool( "usage",     m_usage_column );
    m_skeleton->addItemBool( "group",     m_group_column );
    m_skeleton->addItemBool( "flags",     m_flags_column );
    m_skeleton->addItemBool( "mount",     m_mount_column );

    m_device_check    = new QCheckBox("Device name");
    m_partition_check = new QCheckBox("Partition type");
    m_capacity_check  = new QCheckBox("Capacity");
    m_used_check      = new QCheckBox("Space used");
    m_usage_check     = new QCheckBox("Usage of device");
    m_group_check     = new QCheckBox("Volume group");
    m_flags_check     = new QCheckBox("Partition flags");
    m_mount_check     = new QCheckBox("Mount point");
    m_device_check->setChecked(m_device_column);
    m_partition_check->setChecked(m_partition_column);
    m_capacity_check->setChecked(m_capacity_column);
    m_used_check->setChecked(m_used_column);
    m_usage_check->setChecked(m_usage_column);
    m_group_check->setChecked(m_group_column);
    m_flags_check->setChecked(m_flags_column);
    m_mount_check->setChecked(m_mount_column);

    device_layout->addWidget(m_device_check);
    device_layout->addWidget(m_partition_check);
    device_layout->addWidget(m_capacity_check);
    device_layout->addWidget(m_used_check);
    device_layout->addWidget(m_usage_check);
    device_layout->addWidget(m_group_check);
    device_layout->addWidget(m_flags_check);
    device_layout->addWidget(m_mount_check);

    m_skeleton->setCurrentGroup("VolumeTreeColumns");
    m_skeleton->addItemBool( "volume",      m_volume_column );
    m_skeleton->addItemBool( "size",        m_size_column );
    m_skeleton->addItemBool( "type",        m_type_column );
    m_skeleton->addItemBool( "filesystem",  m_filesystem_column );
    m_skeleton->addItemBool( "stripes",     m_stripes_column );
    m_skeleton->addItemBool( "stripesize",  m_stripesize_column );
    m_skeleton->addItemBool( "state",       m_state_column );
    m_skeleton->addItemBool( "access",      m_access_column );
    m_skeleton->addItemBool( "tags",        m_tags_column );
    m_skeleton->addItemBool( "mountpoints", m_mountpoints_column );

    m_volume_check      = new QCheckBox("Volume name");
    m_size_check        = new QCheckBox("Size");
    m_type_check        = new QCheckBox("Volume type");
    m_filesystem_check  = new QCheckBox("Filesystem type");
    m_stripes_check     = new QCheckBox("Stripe count");
    m_stripesize_check  = new QCheckBox("Stripe size");
    m_state_check       = new QCheckBox("Volume state");
    m_access_check      = new QCheckBox("Volume access");
    m_tags_check        = new QCheckBox("Tags");
    m_mountpoints_check = new QCheckBox("Mount points");

    m_volume_check->setChecked(m_volume_column);
    m_size_check->setChecked(m_size_column);
    m_type_check->setChecked(m_type_column);
    m_filesystem_check->setChecked(m_filesystem_column);
    m_stripes_check->setChecked(m_stripes_column);
    m_stripesize_check->setChecked(m_stripesize_column);
    m_state_check->setChecked(m_state_column);
    m_access_check->setChecked(m_access_column);
    m_tags_check->setChecked(m_tags_column);
    m_mountpoints_check->setChecked(m_mountpoints_column);

    volume_layout->addWidget(m_volume_check);
    volume_layout->addWidget(m_size_check);
    volume_layout->addWidget(m_type_check);
    volume_layout->addWidget(m_filesystem_check);
    volume_layout->addWidget(m_stripes_check);
    volume_layout->addWidget(m_stripesize_check);
    volume_layout->addWidget(m_state_check);
    volume_layout->addWidget(m_access_check);
    volume_layout->addWidget(m_tags_check);
    volume_layout->addWidget(m_mountpoints_check);

    KPageWidgetItem  *page_widget_item =  addPage( general, "General"); 
    page_widget_item->setIcon( KIcon("configure") );
}

void KvpmConfigDialog::buildColorsPage()
{
    QWidget *colors = new QWidget;
    QVBoxLayout *colors_layout = new QVBoxLayout();
    colors->setLayout(colors_layout);

    QGroupBox *selection_box = new QGroupBox( i18n("Filesystem types") );
    QGridLayout *selection_layout = new QGridLayout();
    selection_box->setLayout(selection_layout);
    colors_layout->addWidget(selection_box);

    KSeparator *left_separator  = new KSeparator( Qt::Vertical );
    KSeparator *right_separator = new KSeparator( Qt::Vertical );
    left_separator->setLineWidth(2);
    right_separator->setLineWidth(2);
    left_separator->setFrameStyle(  QFrame::Sunken | QFrame::StyledPanel );
    right_separator->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );

    selection_layout->addWidget( left_separator,  0, 2, 5, 1 );
    selection_layout->addWidget( right_separator, 0, 5, 5, 1 );

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
    m_skeleton->addItemColor("none",  m_none_color);
    m_skeleton->addItemColor("free",  m_free_color);
    m_skeleton->addItemColor("swap",  m_swap_color);
    m_skeleton->addItemColor("physvol",  m_physical_color);

    QLabel *ext2_label = new QLabel("ext2");
    selection_layout->addWidget(ext2_label, 0, 0, Qt::AlignRight);
    m_ext2_button = new KColorButton( m_ext2_color );
    selection_layout->addWidget(m_ext2_button, 0, 1, Qt::AlignLeft);

    QLabel *ext3_label = new QLabel("ext3");
    selection_layout->addWidget(ext3_label, 1, 0, Qt::AlignRight);
    m_ext3_button = new KColorButton( m_ext3_color );
    selection_layout->addWidget(m_ext3_button, 1, 1, Qt::AlignLeft);

    QLabel *ext4_label = new QLabel("ext4");
    selection_layout->addWidget(ext4_label, 2, 0, Qt::AlignRight);
    m_ext4_button = new KColorButton( m_ext4_color );
    selection_layout->addWidget(m_ext4_button, 2, 1, Qt::AlignLeft);

    QLabel *btrfs_label = new QLabel("btrfs");
    selection_layout->addWidget(btrfs_label, 3, 0, Qt::AlignRight);
    m_btrfs_button = new KColorButton( m_btrfs_color );
    selection_layout->addWidget(m_btrfs_button, 3, 1, Qt::AlignLeft);

    QLabel *reiser_label = new QLabel("reiser");
    selection_layout->addWidget(reiser_label, 0, 3, Qt::AlignRight);
    m_reiser_button = new KColorButton( m_reiser_color );
    selection_layout->addWidget(m_reiser_button, 0, 4, Qt::AlignLeft);

    QLabel *reiser4_label = new QLabel("reiser4");
    selection_layout->addWidget(reiser4_label, 1, 3, Qt::AlignRight);
    m_reiser4_button = new KColorButton( m_reiser4_color );
    selection_layout->addWidget(m_reiser4_button, 1, 4, Qt::AlignLeft);

    QLabel *msdos_label = new QLabel("ms-dos");
    selection_layout->addWidget(msdos_label, 2, 3, Qt::AlignRight);
    m_msdos_button = new KColorButton( m_msdos_color );
    selection_layout->addWidget(m_msdos_button, 2, 4, Qt::AlignLeft);

    QLabel *jfs_label = new QLabel("jfs");
    selection_layout->addWidget(jfs_label, 0, 6, Qt::AlignRight);
    m_jfs_button = new KColorButton( m_jfs_color );
    selection_layout->addWidget(m_jfs_button, 0, 7, Qt::AlignLeft);

    QLabel *xfs_label = new QLabel("xfs");
    selection_layout->addWidget(xfs_label, 1, 6, Qt::AlignRight);
    m_xfs_button = new KColorButton( m_xfs_color );
    selection_layout->addWidget(m_xfs_button, 1, 7, Qt::AlignLeft);

    QLabel *swap_label = new QLabel("linux swap");
    selection_layout->addWidget(swap_label, 2, 6, Qt::AlignRight);
    m_swap_button = new KColorButton( m_swap_color );
    selection_layout->addWidget(m_swap_button, 2, 7, Qt::AlignLeft);

    QLabel *none_label = new QLabel("none");
    selection_layout->addWidget(none_label, 3, 3,  Qt::AlignRight);
    m_none_button = new KColorButton( m_none_color );
    selection_layout->addWidget(m_none_button, 3, 4, Qt::AlignLeft);

    QLabel *free_label = new QLabel("free space");
    selection_layout->addWidget(free_label, 4, 3, Qt::AlignRight);
    m_free_button = new KColorButton( m_free_color );
    selection_layout->addWidget(m_free_button, 4, 4, Qt::AlignLeft);

    QLabel *hfs_label = new QLabel("hfs");
    selection_layout->addWidget(hfs_label, 3, 6,  Qt::AlignRight);
    m_hfs_button = new KColorButton( m_hfs_color );
    selection_layout->addWidget(m_hfs_button, 3, 7, Qt::AlignLeft);

    QLabel *physical_label = new QLabel("physical volumes");
    selection_layout->addWidget(physical_label, 4, 0,  Qt::AlignRight);
    m_physical_button = new KColorButton( m_physical_color );
    selection_layout->addWidget(m_physical_button, 4, 1, Qt::AlignLeft);

    KPageWidgetItem  *page_widget_item =  addPage( colors, "Colors"); 
    page_widget_item->setIcon( KIcon("color-picker") );
}

void KvpmConfigDialog::buildProgramsPage()
{
    m_executables_table = new QTableWidget();

    KTabWidget  *programs = new KTabWidget;
    QVBoxLayout *programs1_layout = new QVBoxLayout();
    QVBoxLayout *programs2_layout = new QVBoxLayout();
    QWidget *programs1 = new QWidget();
    QWidget *programs2 = new QWidget();
    programs1->setLayout(programs1_layout);
    programs2->setLayout(programs2_layout);

    m_skeleton->setCurrentGroup("SystemPaths");
    m_skeleton->addItemStringList("SearchPath", m_search_entries, QStringList() );

    m_edit_list = new KEditListBox();
    programs1_layout->addWidget(m_edit_list);

    programs->insertTab( 1, programs1, "Search path");
    m_edit_list->insertStringList( m_search_entries );

    programs2_layout->addWidget(m_executables_table);
    programs->insertTab( 1, programs2, "Executables");

    fillExecutablesTable();

    KPageWidgetItem  *page_widget_item =  addPage( programs, "Programs"); 
    page_widget_item->setIcon( KIcon("applications-system") );
} 

void KvpmConfigDialog::updateSettings()
{
    m_search_entries = m_edit_list->items();

    for( int x = 0; x < m_search_entries.size(); x++)
      if( ! m_search_entries[x].endsWith("/") )
	m_search_entries[x].append("/");

    m_ext2_color  = m_ext2_button->color();
    m_ext3_color  = m_ext3_button->color();
    m_ext4_color  = m_ext4_button->color();
    m_btrfs_color = m_btrfs_button->color();
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
    m_used_column      = m_used_check->isChecked();
    m_usage_column     = m_usage_check->isChecked();
    m_group_column     = m_group_check->isChecked();
    m_flags_column     = m_flags_check->isChecked();
    m_mount_column     = m_mount_check->isChecked();

    m_volume_column      = m_volume_check->isChecked();
    m_size_column        = m_size_check->isChecked();
    m_type_column        = m_type_check->isChecked();
    m_filesystem_column  = m_filesystem_check->isChecked();
    m_stripes_column     = m_stripes_check->isChecked();
    m_stripesize_column  = m_stripesize_check->isChecked();
    m_state_column       = m_state_check->isChecked();
    m_access_column      = m_access_check->isChecked();
    m_tags_column        = m_tags_check->isChecked();
    m_mountpoints_column = m_mountpoints_check->isChecked();

    m_skeleton->writeConfig();

    g_executable_finder->reload();
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
    m_jfs_button->setColor(Qt::magenta);
    m_msdos_button->setColor(Qt::darkYellow);
    m_reiser_button->setColor(Qt::red);
    m_reiser4_button->setColor(Qt::darkRed);
    m_physical_button->setColor(Qt::darkGreen);

    m_device_check->setChecked(true);
    m_partition_check->setChecked(true);
    m_capacity_check->setChecked(true);
    m_used_check->setChecked(true);
    m_usage_check->setChecked(true);
    m_group_check->setChecked(true);
    m_flags_check->setChecked(true);
    m_mount_check->setChecked(true);

    m_volume_check->setChecked(true);
    m_size_check->setChecked(true);
    m_type_check->setChecked(true);
    m_filesystem_check->setChecked(true);
    m_stripes_check->setChecked(false);
    m_stripesize_check->setChecked(false);
    m_state_check->setChecked(true);
    m_access_check->setChecked(false);
    m_tags_check->setChecked(true);
    m_mountpoints_check->setChecked(true);
}

void KvpmConfigDialog::fillExecutablesTable()
{
    QTableWidgetItem *table_widget_item = NULL;

    QStringList all_names = g_executable_finder->getAllNames();
    QStringList all_paths = g_executable_finder->getAllPaths();
    QStringList not_found = g_executable_finder->getNotFound();
    int not_found_length = not_found.size();

    m_executables_table->clear();
    m_executables_table->setColumnCount(2);
    m_executables_table->setRowCount( all_names.size() + not_found_length );

    for(int x = 0; x < not_found_length; x++){
        table_widget_item = new QTableWidgetItem( not_found[x] );
	m_executables_table->setItem(x, 0, table_widget_item);

	table_widget_item = new QTableWidgetItem( KIcon("dialog-error"), "Not Found" );
	m_executables_table->setItem(x, 1, table_widget_item);
    }

    // these lists should be the same length, but just in case...

    for(int x = 0; (x < all_names.size()) && (x < all_paths.size()); x++){

        table_widget_item = new QTableWidgetItem( all_names[x] );
	m_executables_table->setItem(x + not_found_length, 0, table_widget_item);

	table_widget_item = new QTableWidgetItem( all_paths[x] );
	m_executables_table->setItem(x + not_found_length, 1, table_widget_item);

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
