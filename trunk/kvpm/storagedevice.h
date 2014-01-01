/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
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

#include <parted/parted.h>

#include <QObject>
#include <QStringList>

#include "storagebase.h"

class MountTables;
class PhysVol;
class StoragePartition;

class StorageDevice : public StorageBase
{
    long long m_device_size; // Size in bytes
    QString   m_disk_label;
    QString   m_hardware;
    long long m_physical_sector_size;
    int       m_freespace_count;

    QList<StoragePartition *> m_storage_partitions;

public:
    StorageDevice(PedDevice *const pedDevice, 
                  const QList<PhysVol *> pvList, 
                  MountTables *const tables, 
                  const QStringList dmblock, 
                  const QStringList dmraid,
                  const QStringList mdblock, 
                  const QStringList mdraid);
    ~StorageDevice();
        
    QString getDiskLabel() const { return m_disk_label; }
    QString getHardware() const { return m_hardware; }
    QList<StoragePartition *> getStoragePartitions() const { return m_storage_partitions; }
    int getPartitionCount() const { return m_storage_partitions.size(); }
    int getRealPartitionCount() const { return m_storage_partitions.size() - m_freespace_count; }
    long long getSize() const { return m_device_size; }               // Size in bytes
    long long getPhysicalSectorSize() const { return m_physical_sector_size; }
};

#endif
