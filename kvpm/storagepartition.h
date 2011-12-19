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
class MountEntry;
class MountTables;

class StoragePartition 
{
    QList<MountEntry *> m_device_mount_info_list;
    QString m_fstab_mount_point;
    PhysVol *m_pv;
    PedPartition *m_ped_partition;
    QString m_partition_path;
    QString m_partition_type;
    unsigned int m_ped_type;
    QString m_fs_type;
    QString m_fs_uuid;
    QString m_fs_label;
    QStringList m_flags;
    long long m_partition_size;
    long long m_first_sector;
    long long m_last_sector;
    long long m_fs_size;
    long long m_fs_used;
    int m_major;            // block dev numbers
    int m_minor;
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
		     MountTables *mountInfoList);

    ~StoragePartition();

    PedPartition *getPedPartition();    
    QString getFilesystem();
    QString getFilesystemUuid();
    QString getFilesystemLabel();
    QString getName();
    PhysVol *getPhysicalVolume();
    QString getType();
    unsigned int getPedType();
    QString getFstabMountPoint();
    QStringList getMountPoints();
    QStringList getFlags();
    QList<int> getMountPosition();
    long long getSize();
    long long getFirstSector();
    long long getLastSector();
    long long getFilesystemSize();
    long long getFilesystemUsed();
    long long getFilesystemRemaining();
    int getFilesystemPercentUsed();
    int getMajorNumber();
    int getMinorNumber();
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
