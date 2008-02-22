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
#ifndef UNMOUNT_H
#define UNMOUNT_H

#include <KDialog>
#include <QStringList>
#include <QList>

class LogVol;
class NoMungeCheck;
class StoragePartition;

class UnmountDialog : public KDialog
{
Q_OBJECT

    QList<NoMungeCheck *> check_list; // one check box for each mount point
    QStringList mount_points;
 
 public:
    UnmountDialog(QString Device, QStringList MountPoints, QWidget *parent = 0);

 private slots:   
    void unmountFilesystems();
 
};

bool unmount_filesystem(StoragePartition *Partition);
bool unmount_filesystem(LogVol *LogicalVolume);
bool unmount_filesystem(const QString MountPoint);

#endif
