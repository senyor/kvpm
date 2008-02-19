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
    long long partition_size;
    
    PedPartition *part = NULL;
    PedDisk      *disk = NULL;
    PedPartitionType  part_type;
    
    m_device_size = (pedDevice->length) * 512;
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
	    
	    if( !(part_type & 0x08) ) {

		partition_size = ((part->geom).length) * 512;

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
		    partition_type = "logical";
		    partition_path = "freespace"; 
		}
		else{
		    partition_type = "normal";
		    partition_path = "freespace"; 
		}
		m_storage_partitions.append(new StoragePartition( partition_path,
								partition_type,
								partition_size,
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

bool StorageDevice::isPhysicalVolume()
{
    return m_physical_volume;
}

PhysVol* StorageDevice::getPhysicalVolume()
{
    return m_pv;
}

