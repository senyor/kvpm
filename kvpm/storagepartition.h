/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Klvm project.
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
    QList<MountInformation *> device_mount_info_list;
    PhysVol *pv;
    
    int num;
    QString partition_scheme;
    QString type_string;
    
    QString partition_path;
    QString partition_type;
    QString fs_type;
    QString volume_group;
    QString pv_uuid;
    long long partition_size;
    bool physical_volume;
    bool mounted;
    bool mountable;
    
public: 
    StoragePartition(QString PartitionPath,
		     QString PartitionType,
		     long long PartitionSize, 
		     QList<PhysVol *> pv_list, 
		     MountInformationList *mount_info_list);
    ~StoragePartition();
    
    int getNumber();
    QString getFileSystem();
    QString getPartitionPath();
    PhysVol *getPhysicalVolume();
    QString getType();
    QStringList getMountPoints();
    long long getPartitionSize();
    bool isPV();
    bool isMounted();
    bool isMountable();
};

#endif
