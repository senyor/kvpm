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
    scanVolumeGroups();
    scanLogicalVolumes();
    scanPhysicalVolumes();
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

    for(int x = 0; x < m_physical_volumes.size(); x++)
	delete m_physical_volumes[x];

    for(int x = 0; x < m_storage_devices.size(); x++)
	delete m_storage_devices[x];
}

/* The next function takes a VolGroup object as its parameter
   then rescans the information for the volume group it represents.
   The resulting VolGroup object is placed in the m_volume_groups
   list at the location of the original, which is then deleted. 
   This function is only useful when the operation, such a changing
   logical volume attributes, is one that won't effect other groups 
   or change information on the first tab of the main display page. */


VolGroup* MasterList::rebuildVolumeGroup(VolGroup *volumeGroup)
{
    VolGroup *new_group, *old_group;

    old_group = volumeGroup;
    new_group = scanVolumeGroups( old_group->getName() );

    for(int x = 0; x <  m_volume_groups.size(); x++){
	if(m_volume_groups[x] == old_group)
	    m_volume_groups[x] = new_group;
    }

    delete old_group;

    scanLogicalVolumes(new_group);
    scanPhysicalVolumes(new_group);

    return new_group;
}

void MasterList::scanVolumeGroups()
{
    QStringList vg_output;
    QStringList arguments;
    int vg_count;
    
    /* First we run "vgs" to find out what volume groups
       we have and get some information on them  */
    
    arguments << "vgs" << "-o" 
	      <<  "+vg_extent_size,vg_extent_count,vg_free_count,vg_fmt,max_pv,max_lv"  
	      << "--noheadings" << "--separator" 
	      << "|" << "--nosuffix" 
	      << "--units" << "b" 
	      << "--partial"; 
    
    ProcessProgress vgscan(arguments, i18n("Scanning volume groups") );
    vg_output = vgscan.programOutput();
    vg_count = vg_output.size();
    
    for(int x = 0 ; x < vg_count ; x++)
	m_volume_groups.append(new VolGroup(vg_output[x]));
}

VolGroup* MasterList::scanVolumeGroups(QString VolumeGroupName)
{
    QStringList vg_output;
    QStringList arguments;
    
    /* Running vgs gets us the information we
       need to have on the volume group named  */
    
    arguments << "vgs" << "-o" 
	      <<  "+vg_extent_size,vg_extent_count,vg_free_count,vg_fmt,max_pv,max_lv"  
	      << "--noheadings" << "--separator" 
	      << "|" << "--nosuffix" 
	      << "--units" << "b" 
	      << "--partial" << VolumeGroupName; 
    
    ProcessProgress vgscan(arguments, i18n("Scanning volume groups") );
    vg_output = vgscan.programOutput();
    
    return (new VolGroup(vg_output[0]));
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
	      << "lv_name,vg_name,lv_attr,lv_size,origin,snap_percent,move_pv,mirror_log,copy_percent,chunksize,seg_count,stripes,stripesize,seg_size,devices,lv_kernel_major,lv_kernel_minor,lv_minor"  
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
	      << "lv_name,vg_name,lv_attr,lv_size,origin,snap_percent,move_pv,mirror_log,copy_percent,chunksize,seg_count,stripes,stripesize,seg_size,devices,lv_kernel_major,lv_kernel_minor,lv_minor"  
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

	  
/*  Once the second program 'lvs' completes we start up
    the last one 'pvs' and then get its output    */

void MasterList::scanPhysicalVolumes()
{
    QStringList pv_output;
    QStringList arguments;
    int pv_count;
    int vg_count = getVolGroupCount();
    
    arguments << "pvs" 
	      << "--noheadings" 
	      << "--separator" << "|" 
	      << "--nosuffix" 
	      << "--units" << "b" 
	      << "--partial" 
	      << "-o" << "+pv_used,pv_uuid";
    
    ProcessProgress pvscan(arguments, i18n("Scanning physical volumes...") );
    pv_output = pvscan.programOutput();
    
    pv_count = pv_output.size();

    for(int x = 0; x < m_physical_volumes.size(); x++) // Delete everything in the old list
	delete m_physical_volumes[x];

    m_physical_volumes.clear();
    
    for(int x = 0; x < pv_count; x++)
	m_physical_volumes.append(new PhysVol(pv_output[x]));

    for(int vg = 0; vg < vg_count; vg++){
	m_volume_groups[vg]->clearPhysicalVolumes();
	for(int pv = 0; pv < pv_count; pv++){
	    if(m_physical_volumes[pv]->getVolumeGroupName() == m_volume_groups[vg]->getName()){
		m_volume_groups[vg]->addPhysicalVolume(m_physical_volumes[pv]);
	    }
	}
    }
}

void MasterList::scanPhysicalVolumes(VolGroup *VolumeGroup)
{
    PhysVol *physical_volume;
    QString group_name;
    QStringList pv_output;
    QStringList arguments;
    int pv_count;

    group_name = VolumeGroup->getName();
    
    arguments << "pvs" 
	      << "--noheadings" 
	      << "--separator" << "|" 
	      << "--nosuffix" 
	      << "--units" << "b" 
	      << "--partial" 
	      << "-o" << "+pv_used,pv_uuid";
    
    ProcessProgress pvscan(arguments, i18n("Scanning physical volumes...") );
    pv_output = pvscan.programOutput();
    
    pv_count = pv_output.size();

    for(int x = m_physical_volumes.size() - 1; x >= 0; x--) // Delete old list entries for this group
	if(m_physical_volumes[x]->getVolumeGroupName() == group_name){
	    delete m_physical_volumes[x];
	    m_physical_volumes.removeAt(x);
	}
    
    for(int x = 0; x < pv_count; x++){
	physical_volume = new PhysVol(pv_output[x]);

	if(physical_volume->getVolumeGroupName() == group_name){
	    VolumeGroup->addPhysicalVolume(physical_volume);
	    m_physical_volumes.append(physical_volume);
	}
	else
	    delete physical_volume;
    }
}

void MasterList::scanStorageDevices()
{
    PedDevice *dev = NULL;
    
    for(int x = 0; x < m_storage_devices.size(); x++)
	delete m_storage_devices[x];

    m_storage_devices.clear();

    ped_device_probe_all();

    MountInformationList *mount_info_list = new MountInformationList();

    while( ( dev = ped_device_get_next(dev) ) ){
	m_storage_devices.append( new StorageDevice(dev, m_physical_volumes, mount_info_list ) );
    }
}

int MasterList::getVolGroupCount()
{
      return m_volume_groups.size();
}

int MasterList::getPhysVolCount()
{
      return m_physical_volumes.size();
}

const QList<VolGroup *> MasterList::getVolGroups()
{
      return m_volume_groups;
}

const QList<PhysVol *> MasterList::getPhysVols()
{
    return m_physical_volumes;
}

const QList<StorageDevice *> MasterList::getStorageDevices()
{
    return m_storage_devices;
}

PhysVol* MasterList::getPhysVolByName(QString name)
{
    name = name.trimmed();

    for(int x = 0; x < m_physical_volumes.size(); x++){
	if(name == m_physical_volumes[x]->getDeviceName())
	    return m_physical_volumes[x];
    }

    return NULL;
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
