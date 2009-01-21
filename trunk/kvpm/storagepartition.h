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

#include <parted/parted.h>


class PhysVol;
class MountInformation;
class MountInformationList;


class StoragePartition 
{
    QList<MountInformation *> m_device_mount_info_list;
    PhysVol *m_pv;
    PedPartition *m_ped_partition;
    QString m_partition_path;
    QString m_partition_type;
    QString m_fs_type;
    QStringList m_flags;
    long long m_partition_size;
    long long m_first_sector;
    long long m_last_sector;
    bool m_is_pv;
    bool m_is_mounted;
    bool m_is_busy;
    bool m_is_mountable;
    
public: 
    StoragePartition(PedPartition *part,
		     int freespaceCount,
		     QList<PhysVol *> pvList, 
		     MountInformationList *mountInfoList);

    ~StoragePartition();
    
    PedPartition *getPedPartition();
    QString getFileSystem();
    QString getPartitionPath();
    PhysVol *getPhysicalVolume();
    QString getType();
    QStringList getMountPoints();
    QStringList getFlags();
    QList<int> getMountPosition();
    long long getPartitionSize();
    long long getFirstSector();
    long long getLastSector();
    bool isPV();
    bool isMounted();
    bool isBusy();
    bool isMountable();
};

#endif
