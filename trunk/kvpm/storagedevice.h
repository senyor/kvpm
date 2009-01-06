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


#ifndef STORAGEDEVICE_H
#define STORAGEDEVICE_H

#include <parted/parted.h>

#include <QWidget>
#include <QString>

#include "mountinfo.h"

class PhysVol;
class StoragePartition;


class StorageDevice : public QObject
{
    long long m_device_size;
    QString   m_device_path;
    QString   m_disk_label;
    QString   m_hardware;
    long long m_sector_size;
    long long m_physical_sector_size;
    bool      m_readonly;
    bool      m_physical_volume;
    PhysVol  *m_pv;

    QList<StoragePartition *> m_storage_partitions;

 public:
    StorageDevice(PedDevice *pedDevice,
		  QList<PhysVol *> pvList, 
		  MountInformationList *mountInformationList);

    QString getDevicePath();
    QString getDiskLabel();
    QString getHardware();
    QList<StoragePartition *> getStoragePartitions();
    int getPartitionCount();
    long long getSize();
    long long getSectorSize();
    long long getPhysicalSectorSize();
    bool isReadOnly();
    bool isPhysicalVolume();
    PhysVol *getPhysicalVolume();
    
};

#endif
