/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
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

#include <parted/parted.h>

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
    PedPartition *m_ped_partition;
    QString m_partition_path;
    QString m_partition_type;
    unsigned int m_ped_type;
    QString m_fs_type;
    QStringList m_flags;
    long long m_partition_size;
    long long m_first_sector;
    long long m_last_sector;
    long long m_fs_size;
    long long m_fs_used;
    long long m_block_size;   // filesystem block size
    bool m_is_writable;
    bool m_is_pv;
    bool m_is_mounted;
    bool m_is_empty;
    bool m_is_busy;
    bool m_is_mountable;
    bool m_is_normal;
    bool m_is_logical;
    
public: 
    StoragePartition(PedPartition *part,
		     int freespaceCount,
		     QList<PhysVol *> pvList, 
		     MountInformationList *mountInfoList);

    ~StoragePartition();

    PedPartition *getPedPartition();    
    QString getFilesystem();
    QString getName();
    PhysVol *getPhysicalVolume();
    QString getType();
    unsigned int getPedType();
    QStringList getMountPoints();
    QStringList getFlags();
    QList<int> getMountPosition();
    long long getSize();
    long long getFirstSector();
    long long getLastSector();
    long long getFilesystemSize();
    long long getFilesystemUsed();
    long long getFilesystemBlockSize();
    int getPercentUsed();
    bool isWritable();
    bool isPhysicalVolume();
    bool isMounted();
    bool isEmpty();
    bool isBusy();
    bool isMountable();
    bool isNormal();
    bool isLogical();
};

#endif
