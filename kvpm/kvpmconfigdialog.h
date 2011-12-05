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

#ifndef KVPMCONFIGDIALOG_H
#define KVPMCONFIGDIALOG_H

#include <KConfigDialog>
#include <KConfigSkeleton>
#include <KColorButton>
#include <KEditListBox>

#include <QColor>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QStringList>
#include <QSpinBox>
#include <QTableWidget>
#include <QWidget>

class ExecutableFinder;


class KvpmConfigDialog: public KConfigDialog
{
Q_OBJECT

    QTableWidget     *m_executables_table;
    KEditListBox     *m_edit_list;
    KConfigSkeleton  *m_skeleton;
    QStringList       m_search_entries;
    ExecutableFinder *m_executable_finder;

    QSpinBox        *m_fs_warn_spin,
                    *m_pv_warn_spin;

    int m_fs_warn_percent, 
        m_pv_warn_percent;

    KColorButton *m_ext2_button,
                 *m_ext3_button,
                 *m_ext4_button,
                 *m_btrfs_button,
                 *m_reiser_button,
                 *m_reiser4_button,
                 *m_msdos_button,
                 *m_swap_button,
                 *m_xfs_button,
                 *m_jfs_button,
                 *m_hfs_button,
                 *m_ntfs_button,
                 *m_free_button,
                 *m_none_button,
                 *m_physical_button;

    QColor m_ext2_color,
           m_ext3_color,
           m_ext4_color,
           m_btrfs_color,
           m_reiser_color,
           m_reiser4_color,
           m_msdos_color,
           m_swap_color,
           m_xfs_color,
           m_jfs_color,
           m_hfs_color,
           m_ntfs_color,
           m_free_color,
           m_none_color,
           m_physical_color;

    QCheckBox *m_device_check,       *m_volume_check,      *m_pvname_check,
              *m_partition_check,    *m_size_check,        *m_pvsize_check,
              *m_capacity_check,     *m_remaining_check,   *m_pvremaining_check,
              *m_devremaining_check, *m_filesystem_check,  *m_pvused_check,
              *m_usage_check,        *m_stripes_check,     *m_pvstate_check,
              *m_group_check,        *m_stripesize_check,  *m_pvallocate_check,
              *m_flags_check,        *m_snapmove_check,    *m_pvtags_check,
              *m_mount_check,        *m_state_check,       *m_pvlvnames_check,
              *m_tags_check,         *m_access_check,      *m_expandparts_check, 
              *m_mountpoints_check,  *m_type_check;

    bool m_device_column,       m_volume_column,     m_pvname_column,
         m_partition_column,    m_size_column,       m_pvsize_column,
         m_capacity_column,     m_type_column,       m_pvremaining_column, 
         m_devremaining_column, m_filesystem_column, m_pvused_column,
         m_usage_column,        m_stripes_column,    m_pvstate_column,
         m_group_column,        m_stripesize_column, m_pvallocate_column,
         m_flags_column,        m_snapmove_column,   m_pvtags_column,
         m_mount_column,        m_state_column,      m_pvlvnames_column,
         m_tags_column,         m_access_column,     m_show_percent,
         m_mountpoints_column,  m_remaining_column,  m_show_total,
         m_remaining_warn,      m_expand_parts;

    QRadioButton *m_percent_radio,
                 *m_total_radio,
                 *m_both_radio;

    QWidget *generalPage();
    QWidget *colorsPage();
    QWidget *programsPage();
    QGroupBox *allGroup();
    QGroupBox *deviceGroup();
    QGroupBox *logicalGroup();
    QGroupBox *physicalGroup();
    QGroupBox *pvPropertiesGroup();
    QGroupBox *lvPropertiesGroup();
    QGroupBox *devicePropertiesGroup();
    QWidget *treesTab();
    QWidget *propertiesTab();

public:

    KvpmConfigDialog( QWidget *parent, const QString name, KConfigSkeleton *const skeleton, ExecutableFinder *const executableFinder );
    ~KvpmConfigDialog();

public slots:
    void updateSettings();
    void updateWidgetsDefault();
    void fillExecutablesTable();
    bool isDefault();
    bool hasChanged();
};

#endif
