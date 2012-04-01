/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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

#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QStringList>

class KIntSpinBox;
class KTabWidget;
class KLineEdit;
class KComboBox;

class VolGroup;
class LogVol;
class StoragePartition;


class MkfsDialog : public KDialog
{
    Q_OBJECT

    KTabWidget  *m_tab_widget;

    QGroupBox   *m_stripe_box,       *m_base_options_box,
                *m_ext4_options_box, *m_misc_options_box;

    QRadioButton *ext2, *ext3, *ext4, *reiser, *reiser4, *ntfs,
                 *jfs,  *xfs,  *vfat, *swap,   *btrfs;

    KIntSpinBox *m_reserved_spin;  // space reserved for root processes
    KComboBox *m_block_combo;      // blocksize
    KComboBox *m_inode_combo;      // inode size
    KLineEdit *m_name_edit;        // volume name
    KLineEdit *m_inode_edit;       // bytes / inode
    KLineEdit *m_total_edit;       // total inode count
    KLineEdit *m_stride_edit;      // stride size
    KLineEdit *m_count_edit;       // strides per stripe
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
    bool m_bailout;

    QWidget *generalTab();
    QWidget *advancedTab(const long strideSize, const long strideCount);
    QWidget *ext4Tab();
    QGroupBox *miscOptionsBox();
    QGroupBox *baseOptionsBox();
    QGroupBox *ext4OptionsBox();
    QGroupBox *stripeBox(const long strideSize, const long strideCount);
    void clobberFilesystem();
    bool hasInitialErrors(const bool mounted);
    void buildDialog(const long strideSize, const long strideCount);

private slots:
    void enableOptions(bool);
    void commitFilesystem();

public:
    explicit MkfsDialog(LogVol *const volume, QWidget *parent = 0);
    explicit MkfsDialog(StoragePartition *const partition, QWidget *parent = 0);
    bool bailout();

};

#endif
