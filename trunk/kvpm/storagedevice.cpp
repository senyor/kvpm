/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012  Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include <parted/parted.h>


#include "storagedevice.h"
#include "storagepartition.h"

#include "physvol.h"
#include "mounttables.h"


StorageDevice::StorageDevice(PedDevice *const pedDevice,
                             const QList<PhysVol *> pvList,
                             MountTables *const mountTables) : QObject()
{
    PedPartition *part = NULL;
    PedDisk      *disk = NULL;
    PedGeometry       geometry;
    PedPartitionType  part_type;
    long long length;

    m_freespace_count = 0;

    m_sector_size = pedDevice->sector_size;
    m_physical_sector_size = pedDevice->phys_sector_size;
    m_hardware = QString(pedDevice->model);

    m_device_size = (pedDevice->length) * m_sector_size;
    m_device_path = QString("%1").arg(pedDevice->path);

    if (pedDevice->read_only)
        m_is_writable = false;
    else
        m_is_writable = true;

    m_is_busy = ped_device_is_busy(pedDevice);

    m_is_pv = false;
    m_pv = NULL;

    for (int x = 0; x < pvList.size(); x++) {
        if (m_device_path == pvList[x]->getName()) {
            m_pv = pvList[x];
            m_is_pv = true;
        }
    }

    disk = ped_disk_new(pedDevice);

    if (disk && !m_is_pv) {

        PedDiskFlag cylinder_flag = ped_disk_flag_get_by_name("cylinder_alignment");
        if (ped_disk_is_flag_available(disk, cylinder_flag)) {
            ped_disk_set_flag(disk, cylinder_flag, 0);
        }

        m_disk_label = QString(disk->type->name);
        while ((part = ped_disk_next_partition(disk, part))) {

            geometry = part->geom;
            length = geometry.length * m_sector_size;
            part_type = part->type;

            // ignore freespace less than 3 megs
            if (!((part_type & PED_PARTITION_METADATA) || ((part_type & PED_PARTITION_FREESPACE) && (length < (0x300000))))) {
                if (part_type & PED_PARTITION_FREESPACE)
                    m_freespace_count++;

                m_storage_partitions.append(new StoragePartition(part, m_freespace_count, pvList, mountTables));
            }
        }
    } else if (m_is_pv)
        m_disk_label = "physical volume";
    else
        m_disk_label = "unknown";
}

StorageDevice::~StorageDevice()
{
    if (!m_storage_partitions.isEmpty()) {
        for (int x = m_storage_partitions.size() - 1; x >= 0; x--)
            delete m_storage_partitions[x];
    }
}

QString StorageDevice::getName()
{
    return m_device_path;
}

QString StorageDevice::getDiskLabel()
{
    return m_disk_label;
}

QString StorageDevice::getHardware()
{
    return m_hardware;
}

QList<StoragePartition *> StorageDevice::getStoragePartitions()
{
    return m_storage_partitions;
}

int StorageDevice::getPartitionCount()
{
    return m_storage_partitions.size();
}

int StorageDevice::getRealPartitionCount()
{
    return m_storage_partitions.size() - m_freespace_count;
}

long long StorageDevice::getSize()
{
    return m_device_size;
}

long long StorageDevice::getSectorSize()
{
    return m_sector_size;
}

long long StorageDevice::getPhysicalSectorSize()
{
    return m_physical_sector_size;
}

bool StorageDevice::isWritable()
{
    return m_is_writable;
}

bool StorageDevice::isBusy()
{
    return m_is_busy;
}

bool StorageDevice::isPhysicalVolume()
{
    return m_is_pv;
}

PhysVol* StorageDevice::getPhysicalVolume()
{
    return m_pv;
}

