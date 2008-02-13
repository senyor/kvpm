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

#include "fsprobe.h"
#include "mountentry.h"
#include "mountinfo.h"
#include "physvol.h"
#include "storagepartition.h"


StoragePartition::StoragePartition(QString PartitionPath,
				   QString PartitionType,
				   long long PartitionSize, 
				   QList<PhysVol *> pv_list, 
				   MountInformationList *mount_info_list):
    partition_path( PartitionPath ),
    partition_type( PartitionType ),
    partition_size( PartitionSize )

{

    physical_volume = FALSE;
    pv = NULL;
    
    for(int x = 0; x < pv_list.size(); x++){
	if(partition_path == pv_list[x]->getDeviceName()){
	    physical_volume = TRUE;
	    pv = pv_list[x];
	    volume_group = pv->getVolumeGroupName();
	    pv_uuid = pv->getUuid();
	}
    }
    
    device_mount_info_list = mount_info_list->getMountInformation(partition_path);
    if( device_mount_info_list.size() )
	mounted = TRUE;
    else
	mounted = FALSE;

    if( partition_type == "extended" ){
	mountable = FALSE;
	fs_type = "";
    }
    else{
	fs_type = fsprobe_getfstype2(partition_path);
	if( fs_type == "swap" || fs_type == "" )
	    mountable = FALSE;
	else
	    mountable = TRUE;
    }
}

StoragePartition::~StoragePartition()
{
    for(int x = 0; x < device_mount_info_list.size(); x++)
	delete device_mount_info_list[x];
}

int StoragePartition::getNumber()
{
    return num;
}

QString StoragePartition::getType()
{
    return partition_type;
}

QString StoragePartition::getFileSystem()
{
    return fs_type;
}

PhysVol* StoragePartition::getPhysicalVolume()
{
    return pv;
}


QString StoragePartition::getPartitionPath()
{
    return partition_path;
}


long long StoragePartition::getPartitionSize()
{
    return partition_size;
}

bool StoragePartition::isMounted()
{
    return mounted;
}

bool StoragePartition::isMountable()
{
    return mountable;
}

bool StoragePartition::isPV()
{
    return physical_volume;
}


QStringList StoragePartition::getMountPoints()
{
    QStringList mount_points;
    
    for(int x = 0; x < device_mount_info_list.size(); x++)
	mount_points.append( device_mount_info_list[x]->getMountPoint() );

    return mount_points;
}
