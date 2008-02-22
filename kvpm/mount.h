/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
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

#include <KLineEdit>
#include <KDialog>
#include <QString>
#include <QRadioButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGroupBox>

class LogVol;
class StoragePartition;

class MountDialog : public KDialog
{
Q_OBJECT

    QString mount_point,                // The desired mount point 
            device_to_mount,            // The complete device path
            filesystem_type;            // ext3, reiserfs, vfat etc.
 
    QRadioButton *ext2_button, *ext3_button, *reiserfs3_button, *reiserfs4_button,
	         *xfs_button, *jfs_button, *vfat_button, *specify_button,
	         *udf_button, *iso9660_button, *hfs_button,
	         *atime_button, *noatime_button, *nodiratime_button, *relatime_button,
	         *data_journal_button, *data_ordered_button, *data_writeback_button;

    QCheckBox *sync_check, *dirsync_check, *rw_check, *suid_check, 
	      *dev_check, *exec_check, *mand_check, *acl_check, 
              *user_xattr_check;

    KLineEdit *mount_point_edit, 
	      *filesystem_edit,            // User chosen filesystem type
	      *fs_specific_edit;           // Additional options such as acl and data=ordered

    QVBoxLayout *main_layout, *options_layout;
    QGroupBox *filesystem_journal_box;
    
    void setupFilesystemBox();
    void setupOptionsTab();
    void setupMountPointBox();
    
 public:
    MountDialog(QString DeviceToMount, QString Filesystem, QWidget *parent = 0);
    
 private slots:
     void selectMountPoint(bool);
     void mountFilesystem();
     void toggleOKButton(const QString);
     void toggleOKButton(bool);
     void toggleAdditionalOptions(bool);
     
};

bool mount_filesystem(StoragePartition *Partition);
bool mount_filesystem(LogVol *VolumeToMount);

#endif
