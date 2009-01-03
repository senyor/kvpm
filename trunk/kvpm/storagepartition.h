/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef STORAGEPARTITION_H
#define STORAGEPARTITION_H

#include <QObject>
#include <QList>
#include <QStringList>

class PhysVol;
class MountInformation;
class MountInformationList;

class StoragePartition 
{
    QList<MountInformation *> m_device_mount_info_list;
    PhysVol *m_pv;
    QString m_partition_path;
    QString m_partition_type;
    QString m_fs_type;
    long long m_partition_size;
    bool m_is_pv;
    bool m_is_mounted;
    bool m_is_mountable;
    
public: 
    StoragePartition(QString partitionPath,
		     QString partitionType,
		     long long partitionSize, 
		     QList<PhysVol *> pvList, 
		     MountInformationList *mountInfoList);
    ~StoragePartition();
    
    QString getFileSystem();
    QString getPartitionPath();
    PhysVol *getPhysicalVolume();
    QString getType();
    QStringList getMountPoints();
    QList<int> getMountPosition();
    long long getPartitionSize();
    bool isPV();
    bool isMounted();
    bool isMountable();
};

#endif
