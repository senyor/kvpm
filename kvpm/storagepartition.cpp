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

#include "fsprobe.h"
#include "mountentry.h"
#include "mountinfo.h"
#include "physvol.h"
#include "storagepartition.h"


StoragePartition::StoragePartition(PedPartition *part,
				   int freespaceCount,
				   QList<PhysVol *> pvList, 
				   MountInformationList *mountInfoList):
    m_ped_partition (part)

{

    long long sector_size;
    m_is_pv = false;
    m_is_normal  = false;
    m_is_logical = false;
    m_pv = NULL;

    PedDisk   *ped_disk   = m_ped_partition->disk;
    PedDevice *ped_device = ped_disk->dev;
    PedGeometry ped_geometry = m_ped_partition->geom;
    m_ped_type = m_ped_partition->type;

    sector_size      = ped_device->sector_size;
    m_first_sector   = (ped_geometry).start;
    m_last_sector    = (ped_geometry).end;
    m_partition_size = (ped_geometry.length) * sector_size; // in bytes

    if( m_ped_type == 0 ){
      m_partition_type = "normal";
      m_is_normal = true;
      m_partition_path = ped_partition_get_path(part);
    }
    else if( m_ped_type & 0x02 ){
      m_partition_type = "extended";
      m_partition_path = ped_partition_get_path(part);
    }
    else if( (m_ped_type & 0x01) && !(m_ped_type & 0x04) ){
      m_partition_type = "logical";
      m_is_logical = true;
      m_partition_path = ped_partition_get_path(part);
    }
    else if( (m_ped_type & 0x01) && (m_ped_type & 0x04) ){
      m_partition_type = "freespace (logical)";
      m_partition_path = ped_partition_get_path(part);
      m_partition_path.chop(1);
      m_partition_path.append( QString("%1").arg(freespaceCount) );
    }
    else{
      m_partition_type = "freespace";
      m_partition_path = ped_partition_get_path(part);
      m_partition_path.chop(1);
      m_partition_path.append( QString("%1").arg(freespaceCount) );
    }

    for(int x = 0; x < pvList.size(); x++){
	if(m_partition_path == pvList[x]->getDeviceName()){
	    m_is_pv = true;
	    m_pv = pvList[x];
	}
    }

    // Iterate though all the possible flags and check each one

    PedPartitionFlag ped_flag = PED_PARTITION_BOOT;

    if( ! m_partition_type.contains("freespace", Qt::CaseInsensitive) ){
        while( ped_flag != 0  ){
	    if( ped_partition_get_flag(m_ped_partition, ped_flag) )
	        m_flags << ped_partition_flag_get_name(ped_flag);
	
	    ped_flag = ped_partition_flag_next( ped_flag );
        }
	if( ! m_flags.size() )
	    m_flags << "";
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

    m_is_busy = ped_partition_is_busy(m_ped_partition);

    PedPartition *temp_part = NULL;
    PedDisk      *temp_disk = ped_disk_new(ped_device);

    if( m_partition_type == "extended" ){
        m_is_empty = true;
	if( temp_disk ){
	    while( (temp_part = ped_disk_next_partition (temp_disk, temp_part)) ){
	        if( (temp_part->type  == PED_PARTITION_LOGICAL) ){
		    m_is_empty = false;
		    break;
		}
	    }
	}
    }
    else
        m_is_empty = false;

    ped_disk_destroy(temp_disk); 
}

StoragePartition::~StoragePartition()
{
    for(int x = 0; x < m_device_mount_info_list.size(); x++)
	delete m_device_mount_info_list[x];
}

QString StoragePartition::getType()
{
    return m_partition_type.trimmed();
}

unsigned int StoragePartition::getPedType()
{
    return m_ped_type;
}

PedPartition* StoragePartition::getPedPartition()
{
    return m_ped_partition;
}

QString StoragePartition::getFileSystem()
{
    return m_fs_type.trimmed();
}

PhysVol* StoragePartition::getPhysicalVolume()
{
    return m_pv;
}


QString StoragePartition::getPartitionPath()
{
    return m_partition_path.trimmed();
}


long long StoragePartition::getPartitionSize()
{
    return m_partition_size;
}

long long StoragePartition::getFirstSector()
{
    return m_first_sector;
}

long long StoragePartition::getLastSector()
{
    return m_last_sector;
}

bool StoragePartition::isMounted()
{
    return m_is_mounted;
}

/* function returns true if the partition is extended 
   and has no logical partitions */

bool StoragePartition::isEmpty()
{
    return m_is_empty;
}

bool StoragePartition::isNormal()
{
    return m_is_normal;
}

bool StoragePartition::isLogical()
{
    return m_is_logical;
}

bool StoragePartition::isBusy()
{
    return m_is_busy;
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

QStringList StoragePartition::getFlags()
{
    return m_flags;
}
