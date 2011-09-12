/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef MKFS_H
#define MKFS_H

#include <KDialog>
#include <KTabWidget>
#include <KLineEdit>
#include <KComboBox>

#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QStringList>
#include <QSpinBox>

class VolGroup;
class LogVol;
class StoragePartition;

bool make_fs(LogVol *logicalVolume);
bool make_fs(StoragePartition *partition);

class MkfsDialog : public KDialog
{
Q_OBJECT

    KTabWidget  *m_tab_widget;
    QGroupBox   *radio_box, *m_stripe_box, *m_misc_box, 
                *m_base_options_box, *m_ext4_options_box;

    QRadioButton *ext2, *ext3, *ext4, *reiser, *reiser4, *ntfs,
                 *jfs,  *xfs,  *vfat, *swap,   *btrfs;

    KComboBox *m_block_combo;      // blocksize
    KComboBox *m_inode_combo;      // inode size
    KLineEdit *m_volume_edit;      // volume name
    KLineEdit *m_inode_edit;       // bytes / inode
    KLineEdit *m_total_edit;       // total inode count
    KLineEdit *m_stride_edit;      // stride size
    KLineEdit *m_count_edit;       // strides per stripe
    QSpinBox  *m_reserved_spin;
    QCheckBox *m_extent_check;
    QCheckBox *m_ext_attr_check;
    QCheckBox *m_resize_inode_check;
    QCheckBox *m_dir_index_check; 
    QCheckBox *m_filetype_check; 
    QCheckBox *m_sparse_super_check; 
    QCheckBox *m_clobber_fs_check;

    QCheckBox *m_flex_bg_check;  
    QCheckBox *m_huge_file_check;
    QCheckBox *m_uninit_bg_check;
    QCheckBox *m_dir_nlink_check;
    QCheckBox *m_extra_isize_check;
    QCheckBox *m_lazy_itable_init_check;

    QString m_path;

    int m_stride_size, m_stride_count;

    QWidget *generalTab();
    QWidget *advancedTab();

 private slots:
    void clobberFS();
    void setAdvancedTab(bool);

 public:
    explicit MkfsDialog(LogVol *logicalVolume, QWidget *parent = 0);
    explicit MkfsDialog(StoragePartition *partition, QWidget *parent = 0);
    QStringList arguments();
};

#endif
