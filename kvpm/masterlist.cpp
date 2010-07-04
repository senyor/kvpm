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

#include <parted/parted.h>

#include <KLocale>
#include <QtGui>

#include "fsprobe.h"
#include "logvol.h"
#include "masterlist.h"
#include "mountentry.h"
#include "mountinfo.h"
#include "physvol.h"
#include "processprogress.h"
#include "storagedevice.h"
#include "volgroup.h"

MasterList::MasterList() : QObject()
{
    lvm_t  lvm = lvm_init( NULL );
    lvm_scan(lvm);
    scanVolumeGroups(lvm);
    lvm_quit(lvm);

    scanLogicalVolumes();
    scanStorageDevices();
}


/* We can't just clear the lists, we need to delete 
   all the objects they point to  before we can 
   create a brand new list */

MasterList::~MasterList()
{
    for(int x = 0; x < m_volume_groups.size(); x++)
	delete m_volume_groups[x];

    for(int x = 0; x < m_logical_volumes.size(); x++)
	delete m_logical_volumes[x];

    for(int x = 0; x < m_storage_devices.size(); x++)
	delete m_storage_devices[x];
}

void MasterList::scanVolumeGroups(lvm_t lvm)
{
    QStringList vg_output;
    QStringList arguments;

    dm_list *vgnames;
    lvm_str_list *strl;
    
    vgnames = lvm_list_vg_names(lvm);

    dm_list_iterate_items(strl, vgnames)
	m_volume_groups.append( new VolGroup(lvm, strl->str) );
}

void MasterList::scanLogicalVolumes()
{
    QStringList lv_output;
    QStringList seg_data;
    QStringList arguments;
    int lv_count;
    int vg_count;
    int lv_segments;   //number of segments in the logical volume

    MountInformationList mount_info_list;
    
    
    arguments << "lvs" << "--all" << "-o" 
	      << "lv_name,vg_name,lv_attr,lv_size,origin,snap_percent,move_pv,mirror_log,copy_percent,chunksize,seg_count,stripes,stripesize,seg_size,devices,lv_kernel_major,lv_kernel_minor,lv_minor,lv_uuid"  
	      << "--noheadings" << "--separator" 
	      << "|" << "--nosuffix" 
	      << "--units" << "b" 
	      << "--partial"; 
    
    ProcessProgress lvscan(arguments, i18n("Scanning logical volumes...") );
    lv_output = lvscan.programOutput();
    
    for(int i = 0; i < lv_output.size(); ) {
	seg_data.clear();
	lv_segments = (lv_output[i].section('|',10,10)).toInt();

	for(int x = 0 ; x < lv_segments ; x++){
	    seg_data << lv_output[i++];
	}

	m_logical_volumes.append(new LogVol(seg_data, &mount_info_list));

    } 

    /* put pointers to the lvs in their associated vgs */
    
    lv_count = m_logical_volumes.size();
    vg_count = m_volume_groups.size();
    
    for(int vg = 0; vg < vg_count; vg++){
	for(int lv = 0; lv < lv_count; lv++){
	    if(m_logical_volumes[lv]->getVolumeGroupName() == m_volume_groups[vg]->getName()){
		m_volume_groups[vg]->addLogicalVolume(m_logical_volumes[lv]);
		m_logical_volumes[lv]->setVolumeGroup(m_volume_groups[vg]);
	    }
	}    
    }
}

void MasterList::scanLogicalVolumes(VolGroup *VolumeGroup)
{
    LogVol *logical_volume;
    QString group_name;
    QStringList lv_output;
    QStringList seg_data;
    QStringList arguments;
    int lv_segments;   //number of segments in the logical volume
    
    group_name = VolumeGroup->getName();

    MountInformationList mount_info_list;

    arguments << "lvs" << "--all" << "-o" 
	      << "lv_name,vg_name,lv_attr,lv_size,origin,snap_percent,move_pv,mirror_log,copy_percent,chunksize,seg_count,stripes,stripesize,seg_size,devices,lv_kernel_major,lv_kernel_minor,lv_minor,lv_uuid"  
	      << "--noheadings" << "--separator" 
	      << "|" << "--nosuffix" 
	      << "--units" << "b" 
	      << "--partial" << group_name; 
    
    ProcessProgress lvscan(arguments, i18n("Scanning logical volumes...") );
    lv_output = lvscan.programOutput();

    for(int i = 0; i < lv_output.size(); ) {
	seg_data.clear();
	lv_segments = (lv_output[i].section('|',10,10)).toInt();
	
	for(int x = 0 ; x < lv_segments ; x++){
	    seg_data << lv_output[i++];
	}

	logical_volume = new LogVol(seg_data, &mount_info_list);
	VolumeGroup->addLogicalVolume(logical_volume);
	logical_volume->setVolumeGroup(VolumeGroup);
    } 

}

void MasterList::scanStorageDevices()
{
    PedDevice *dev = NULL;
    QList<PhysVol *>  physical_volumes;

    qDebug() << "Scanning storage devices";

    for(int x = 0; x < m_volume_groups.size(); x++)
        physical_volumes.append( m_volume_groups[x]->getPhysicalVolumes() );

    qDebug() << "Scanned storage devices";

    for(int x = 0; x < m_storage_devices.size(); x++)
	delete m_storage_devices[x];

    m_storage_devices.clear();

    ped_device_free_all();
    ped_device_probe_all();

    MountInformationList *mount_info_list = new MountInformationList();

    while( ( dev = ped_device_get_next(dev) ) ){
	m_storage_devices.append( new StorageDevice(dev, physical_volumes, mount_info_list ) );
    }
}

int MasterList::getVolGroupCount()
{
      return m_volume_groups.size();
}

const QList<VolGroup *> MasterList::getVolGroups()
{
      return m_volume_groups;
}

const QList<StorageDevice *> MasterList::getStorageDevices()
{
    return m_storage_devices;
}

VolGroup* MasterList::getVolGroupByName(QString name)
{
    name = name.trimmed();

    for(int x = 0; x < m_volume_groups.size(); x++){
	if(name == m_volume_groups[x]->getName())
	    return m_volume_groups[x];
    }

    return NULL;
}

QStringList MasterList::getVolumeGroupNames()
{
    QStringList names;

    for(int x = 0; x < m_volume_groups.size(); x++)
	names << m_volume_groups[x]->getName();

    return names;
}

