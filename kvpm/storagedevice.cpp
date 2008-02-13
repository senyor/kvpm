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
#include "physvol.h"
#include "mountentry.h"

StorageDevice::StorageDevice( PedDevice *dev,
			      QList<PhysVol *> pv_list, 
			      MountInformationList *mount_info_list) : QObject()
{
    QString partition_path;
    QString partition_type;
    long long partition_size;
    
    PedPartition *part = NULL;
    PedDisk  *disk = NULL;
    PedPartitionType  part_type;
    
    device_size = (dev->length) * 512;
    device_path = QString("%1").arg(dev->path);

    physical_volume = FALSE;
    pv = NULL;

    for(int x = 0; x < pv_list.size(); x++){
	if(device_path == pv_list[x]->getDeviceName()){
	    pv = pv_list[x];
	    volume_group = pv->getVolumeGroupName();
	    physical_volume = TRUE;
	}
    }

    disk = ped_disk_new(dev);
    if( disk && !physical_volume ){
	disk_label = QString( (disk->type)->name );
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
		storage_partitions.append(new StoragePartition( partition_path,
								partition_type,
								partition_size,
								pv_list, 
								mount_info_list ));
	    }
	}
    }
    else if (physical_volume)
	disk_label = "physical volume";
    else
	disk_label = "unknown";
}

QString StorageDevice::getDevicePath()
{
    return device_path;
}

QString StorageDevice::getDiskLabel()
{
    return disk_label;
}

QList<StoragePartition *> StorageDevice::getStoragePartitions()
{
    return storage_partitions;
}

int StorageDevice::getPartitionCount()
{
    return storage_partitions.size();
}

long long StorageDevice::getSize()
{
    return device_size;
}

bool StorageDevice::isPhysicalVolume()
{
    return physical_volume;
}

PhysVol* StorageDevice::getPhysicalVolume()
{
    return pv;
}

QString StorageDevice::getVolumeGroup()
{
    return volume_group;
}

