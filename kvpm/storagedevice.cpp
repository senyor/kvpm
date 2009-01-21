/*
 *
 * 
 * Copyright (C) 2008, 2009 Benjamin Scott   <benscott@nwlink.com>
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
    int freespace_counter = 0;
    long long length;

    m_sector_size = pedDevice->sector_size;
    m_physical_sector_size = pedDevice->phys_sector_size;
    m_hardware = QString(pedDevice->model);

    m_device_size = (pedDevice->length) * m_sector_size;
    m_device_path = QString("%1").arg(pedDevice->path);

    if( pedDevice->read_only )
      m_readonly = true;
    else
      m_readonly = false;

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

	    if( !( (part_type & 0x08) || (  (part_type & 0x04) && (length < (1024 * 1024))))){

	        if( part_type & 0x04 )
		    freespace_counter++;

		m_storage_partitions.append(new StoragePartition( part,
								  freespace_counter,
								  pvList, 
								  mountInformationList ));
	    }
	}
    }
    else if (m_physical_volume)
	m_disk_label = "physical volume";
    else
	m_disk_label = "unknown";
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

bool StorageDevice::isReadOnly()
{
    return m_readonly;
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

