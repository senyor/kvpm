/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
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

#include "fsprobe.h"
#include "mountentry.h"
#include "mountinfo.h"
#include "physvol.h"
#include "storagepartition.h"


StoragePartition::StoragePartition(QString partitionPath,
				   QString partitionType,
				   long long partitionSize, 
				   QList<PhysVol *> pvList, 
				   MountInformationList *mountInfoList):
    m_partition_path( partitionPath ),
    m_partition_type( partitionType ),
    m_partition_size( partitionSize )

{

    m_is_pv = false;
    m_pv = NULL;
    
    for(int x = 0; x < pvList.size(); x++){
	if(m_partition_path == pvList[x]->getDeviceName()){
	    m_is_pv = true;
	    m_pv = pvList[x];
	}
    }
    
    m_device_mount_info_list = mountInfoList->getMountInformation(m_partition_path);

    if( m_device_mount_info_list.size() ){
	m_is_mounted = true;
    }
    else{
	m_is_mounted = false;
    }

    if( m_partition_type == "extended" ){
	m_is_mountable = false;
	m_fs_type = "";
    }
    else{
	m_fs_type = fsprobe_getfstype2(m_partition_path);

	if( m_fs_type == "swap" || m_fs_type == "" ){
	    m_is_mountable = false;
	}
	else{
		m_is_mountable = true;
	}
    }
}

StoragePartition::~StoragePartition()
{
    for(int x = 0; x < m_device_mount_info_list.size(); x++)
	delete m_device_mount_info_list[x];
}

QString StoragePartition::getType()
{
    return m_partition_type;
}

QString StoragePartition::getFileSystem()
{
    return m_fs_type;
}

PhysVol* StoragePartition::getPhysicalVolume()
{
    return m_pv;
}


QString StoragePartition::getPartitionPath()
{
    return m_partition_path;
}


long long StoragePartition::getPartitionSize()
{
    return m_partition_size;
}

bool StoragePartition::isMounted()
{
    return m_is_mounted;
}

bool StoragePartition::isMountable()
{
    return m_is_mountable;
}

bool StoragePartition::isPV()
{
    return m_is_pv;
}

QStringList StoragePartition::getMountPoints()
{
    QStringList mount_points;
    
    for(int x = 0; x < m_device_mount_info_list.size(); x++)
	mount_points.append( m_device_mount_info_list[x]->getMountPoint() );

    return mount_points;
}

QList<int> StoragePartition::getMountPosition()
{
  QList<int> mount_position;
    
    for(int x = 0; x < m_device_mount_info_list.size(); x++)
	mount_position.append( m_device_mount_info_list[x]->getMountPosition() );

    return mount_position;
}
