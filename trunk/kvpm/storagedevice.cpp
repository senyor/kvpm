/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2016  Benjamin Scott   <benscott@nwlink.com>
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
                             MountTables *const tables, 
                             const QStringList dmblock, 
                             const QStringList dmraid,
                             const QStringList mdblock, 
                             const QStringList mdraid) : 
    StorageBase(pedDevice, pvList, dmblock, dmraid, mdblock, mdraid)
{
    const long long sector_size = getSectorSize();
    const bool is_pv = isPhysicalVolume();

    m_freespace_count = 0;
    m_physical_sector_size = pedDevice->phys_sector_size;
    m_hardware = QString(pedDevice->model);
    m_device_size = pedDevice->length * sector_size;
    PedDisk *const disk = ped_disk_new(pedDevice);

    if (disk && !is_pv && !isDmBlock()) {

        PedDiskFlag cylinder_flag = ped_disk_flag_get_by_name("cylinder_alignment");
        if (ped_disk_is_flag_available(disk, cylinder_flag)) {
            ped_disk_set_flag(disk, cylinder_flag, 0);
        }

        m_disk_label = QString(disk->type->name);
        PedPartition *part = nullptr;
    
        while ((part = ped_disk_next_partition(disk, part))) {

            const long long length = part->geom.length * sector_size;
            const PedPartitionType  pt = part->type;

            // ignore freespace less than 3 megs
            if (!((pt & PED_PARTITION_METADATA) || ((pt & PED_PARTITION_FREESPACE) && (length < (0x300000))))) {
                if (pt & PED_PARTITION_FREESPACE)
                    m_freespace_count++;

                m_storage_partitions.append(new StoragePartition(part, m_freespace_count, pvList, tables, mdblock));
            }
        }
    } else if (is_pv) {
        m_disk_label = "physical volume";
    } else {
        m_disk_label = "unknown";
    }
}

StorageDevice::~StorageDevice()
{
    for (auto *part : m_storage_partitions)
        delete part;
}

