/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#ifndef STORAGEDEVICE_H
#define STORAGEDEVICE_H

#include <QWidget>
#include <QString>

#include <parted/parted.h>

class MountTables;
class PhysVol;
class StoragePartition;

class StorageDevice : public QObject
{
Q_OBJECT

    long long m_device_size; // Size in bytes
    QString   m_device_path;
    QString   m_disk_label;
    QString   m_hardware;
    long long m_sector_size;
    long long m_physical_sector_size;
    bool      m_is_writable;
    bool      m_is_busy;
    bool      m_is_pv;
    PhysVol  *m_pv;
    int       m_freespace_count;

    QList<StoragePartition *> m_storage_partitions;

 public:
    StorageDevice(PedDevice * const pedDevice, const QList<PhysVol *> pvList, MountTables *const mountTables);

    QString getName();
    QString getDiskLabel();
    QString getHardware();
    QList<StoragePartition *> getStoragePartitions();
    int getPartitionCount();
    int getRealPartitionCount();
    long long getSize();                // Size in bytes
    long long getSectorSize();
    long long getPhysicalSectorSize();
    bool isWritable();
    bool isBusy();
    bool isPhysicalVolume();
    PhysVol *getPhysicalVolume();
    
};

#endif
