/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef MOUNT_H
#define MOUNT_H

#include "kvpmdialog.h"

class KLineEdit;

class QCheckBox;
class QGroupBox;
class QRadioButton;
class QString;
class QVBoxLayout;

class LogVol;
class StoragePartition;


class MountDialog : public KvpmDialog
{
    Q_OBJECT

    QString m_mount_point,          // The desired mount point
            m_device_to_mount,      // The complete device path
            m_filesystem_type;      // ext3, reiserfs, vfat etc.

    QRadioButton *ext2_button,      *ext3_button,    *ext4_button,       *reiserfs3_button, *btrfs_button,
                 *reiserfs4_button, *xfs_button,     *jfs_button,        *vfat_button,      *ntfs_button,
                 *specify_button,   *udf_button,     *iso9660_button,    *hfs_button,
                 *atime_button,     *noatime_button, *relatime_button,   *data_journal_button,
                 *data_ordered_button, *data_writeback_button;

    QCheckBox *sync_check, *dirsync_check, *rw_check, *suid_check,
              *dev_check, *exec_check, *mand_check, *acl_check,
              *user_xattr_check, *nodiratime_check;

    KLineEdit *m_mount_point_edit,
              *m_filesystem_edit,    // User chosen filesystem type
              *m_fs_specific_edit;   // Additional options such as acl and data=ordered

    QGroupBox *m_filesystem_journal_box;
    bool m_is_writable;

    QWidget* filesystemBox();
    QWidget* mountPointBox();
    QWidget* mainTab();
    QWidget* optionsTab();
    void buildDialog();

public:
    MountDialog(LogVol *const volume, QWidget *parent = nullptr);
    MountDialog(StoragePartition *const partition, QWidget *parent = nullptr);

private slots:
    void selectMountPoint(bool);
    void commit();
    void toggleOKButton();
    void toggleAdditionalOptions(bool);

};


#endif
