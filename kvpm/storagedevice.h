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


#ifndef STORAGEDEVICE_H
#define STORAGEDEVICE_H

#include <parted/parted.h>

#include <QWidget>
#include <QString>

#include "storagepartition.h"
#include "mountinfo.h"

class PhysVol;

class StorageDevice : public QObject
{
    long long device_size;
    QString device_path;
    QString volume_group;
    QString disk_label;
    
    bool physical_volume;
    PhysVol *pv;
    QList<StoragePartition *> storage_partitions;

 public:
    StorageDevice(PedDevice *dev, QList<PhysVol *> pv_list, 
		  MountInformationList *mount_info_list);

    QString getDevicePath();
    QString getDiskLabel();
    QList<StoragePartition *> getStoragePartitions();
    int getPartitionCount();
    long long getSize();
    bool isPhysicalVolume();
    PhysVol *getPhysicalVolume();
    QString getVolumeGroup();
    
};

#endif
