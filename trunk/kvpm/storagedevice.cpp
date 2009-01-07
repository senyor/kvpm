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

#include <QtGui>

#include "storagedevice.h"
#include "storagepartition.h"

#include "physvol.h"
#include "mountentry.h"

StorageDevice::StorageDevice( PedDevice *pedDevice,
			      QList<PhysVol *> pvList, 
			      MountInformationList *mountInformationList) : QObject()
{
    QString   partition_path;
    QString   partition_type;

    int free_space_counter = 1;
    long long partition_size;          // bytes
    long long partition_first_sector;
    long long partition_last_sector;
    
    PedPartition *part = NULL;
    PedDisk      *disk = NULL;
    PedGeometry   geometry;
    PedPartitionType  part_type;

    m_sector_size = pedDevice->sector_size;
    m_physical_sector_size = pedDevice->phys_sector_size;
    m_hardware = QString(pedDevice->model);

    m_device_size = (pedDevice->length) * m_sector_size;
    m_device_path = QString("%1").arg(pedDevice->path);

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

	    part_type = part->type;
	    geometry = part->geom;
	    partition_first_sector = geometry.start;
	    partition_last_sector = geometry.end;

	    if( !(part_type & 0x08) ) {

		partition_size = ((part->geom).length) * m_sector_size;

		if( part_type == 0 ){
		    partition_type = "normal";
		    partition_path = ped_partition_get_path(part);
		}
		else if( part_type & 0x02 ){
		    partition_type = "extended";
		    partition_path = ped_partition_get_path(part);
		}
		else if( (part_type & 0x01) && !(part_type & 0x04) ){
		    partition_type = "logical";
		    partition_path = ped_partition_get_path(part);
		}
		else if( (part_type & 0x01) && (part_type & 0x04) ){
		    partition_type = "freespace (logical)";
		    partition_path = ped_partition_get_path(part);
		    partition_path.chop(1);
		    partition_path.append( QString("%1").arg(free_space_counter++) );
		}
		else{
		    partition_type = "freespace";
		    partition_path = ped_partition_get_path(part);
		    partition_path.chop(1);
		    partition_path.append( QString("%1").arg(free_space_counter++) );
		}
		m_storage_partitions.append(new StoragePartition( partition_path,
								  partition_type,
								  partition_size,
								  partition_first_sector,
								  partition_last_sector,
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

bool StorageDevice::isPhysicalVolume()
{
    return m_physical_volume;
}

PhysVol* StorageDevice::getPhysicalVolume()
{
    return m_pv;
}

