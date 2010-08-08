/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010 Benjamin Scott   <benscott@nwlink.com>
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

#include <QtGui>

#include "storagedevice.h"
#include "storagepartition.h"

#include "physvol.h"
#include "mountentry.h"

StorageDevice::StorageDevice( PedDevice *pedDevice,
			      QList<PhysVol *> pvList, 
			      MountInformationList *mountInformationList) : QObject()
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

    if( pedDevice->read_only )
      m_writable = false;
    else
      m_writable = true;

    m_busy = ped_device_is_busy(pedDevice);

    m_physical_volume = false;
    m_pv = NULL;

    for(int x = 0; x < pvList.size(); x++){
	if(m_device_path == pvList[x]->getDeviceName()){
	    m_pv = pvList[x];
	    m_physical_volume = true;
	}
    }

    disk = ped_disk_new(pedDevice);
    if( disk && !m_physical_volume ){
	m_disk_label = QString( (disk->type)->name );
	while( (part = ped_disk_next_partition (disk, part)) ){

	    geometry  = part->geom;
	    length = geometry.length * m_sector_size;
	    part_type = part->type;

	    if( !( (part_type & 0x08) || (  (part_type & 0x04) && (length < (0x800000))))){  // ignore freespace less than 8megs

	        if( part_type & 0x04 )
		    m_freespace_count++;

		m_storage_partitions.append(new StoragePartition( part,
								  m_freespace_count,
								  pvList, 
								  mountInformationList ));
	    }
	}
    }
    else if (m_physical_volume)
	m_disk_label = "physical volume";
    else
	m_disk_label = "unknown";

    if(disk)
        ped_disk_destroy(disk);
}

QString StorageDevice::getDevicePath()
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
    return m_writable;
}

bool StorageDevice::isBusy()
{
    return m_busy;
}

bool StorageDevice::isPhysicalVolume()
{
    return m_physical_volume;
}

PhysVol* StorageDevice::getPhysicalVolume()
{
    return m_pv;
}

