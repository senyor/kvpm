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

#include <parted/parted.h>

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
    scanVolGroups();
    scanLogVols();
    scanPhysVols();
    scanStorageDevices();
}


/* We can't just clear the lists, we need to delete 
   all the objects they point to  before we can 
   create a brand new list */

MasterList::~MasterList()
{
    for(int x = 0; x < VolGroups.size(); x++)
	delete VolGroups[x];

    for(int x = 0; x < LogVols.size(); x++)
	delete LogVols[x];

    for(int x = 0; x < PhysVols.size(); x++)
	delete PhysVols[x];

    for(int x = 0; x < StorageDevices.size(); x++)
	delete StorageDevices[x];
}

/* The next function takes a VolGroup object as its parameter
   then rescans the information for the volume group it represents.
   The resulting VolGroup object is placed in the VolGroups
   list at the location of the original, which is then deleted. 
   This function is only useful when the operation, such a changing
   logical volume attributes, is one that won't effect other groups 
   or change information on the first tab of the main display page. */


VolGroup* MasterList::rebuildVolumeGroup(VolGroup *VolumeGroup)
{
    VolGroup *new_group, *old_group;

    old_group = VolumeGroup;
    new_group = scanVolGroups(VolumeGroup->getName());

    for(int x = 0; x <  VolGroups.size(); x++)
	if(VolGroups[x] == old_group)
	    VolGroups[x] = new_group;
    delete old_group;

    scanLogVols(new_group);
    scanPhysVols(new_group);

    return new_group;
}

void MasterList::scanVolGroups()
{
    QStringList vg_output;
    QStringList arguments;
    int vg_count;
    
    /* First we run /sbin/vgs to find out what volume groups
       we have and get some information on them  */
    
    arguments << "/sbin/vgs" << "-o" 
	      <<  "+vg_extent_size,vg_extent_count,vg_free_count,vg_fmt,max_pv,max_lv"  
	      << "--noheadings" << "--separator" << "|" << "--nosuffix" 
	      << "--units" << "b" << "--partial"; 
    
    ProcessProgress vgscan(arguments, "Scanning volume groups");
    vg_output = vgscan.programOutput();
    vg_count = vg_output.size();
    
    for(int x = 0 ; x < vg_count ; x++)
	VolGroups.append(new VolGroup(vg_output[x]));
}

VolGroup* MasterList::scanVolGroups(QString VolumeGroupName)
{
    QStringList vg_output;
    QStringList arguments;
    
    /* Running /sbin/vgs gets us the information we
       need to have on the volume group named  */
    
    arguments << "/sbin/vgs" << "-o" 
	      <<  "+vg_extent_size,vg_extent_count,vg_free_count,vg_fmt,max_pv,max_lv"  
	      << "--noheadings" << "--separator" << "|" << "--nosuffix" 
	      << "--units" << "b" << "--partial" << VolumeGroupName; 
    
    ProcessProgress vgscan(arguments, "Scanning volume groups");
    vg_output = vgscan.programOutput();
    
    return (new VolGroup(vg_output[0]));
}

void MasterList::scanLogVols()
{
    QStringList lv_output;
    QStringList seg_data;
    QStringList arguments;
    int lv_count;
    int vg_count;
    int lv_segments;   //number of segments in the logical volume

    mount_info_list = new MountInformationList();
    
    arguments << "/sbin/lvs" << "--all" << "-o" 
	      << "lv_name,vg_name,lv_attr,lv_size,origin,snap_percent,move_pv,mirror_log,copy_percent,chunksize,seg_count,stripes,stripesize,seg_size,devices,lv_kernel_major,lv_kernel_minor,lv_minor"  
	      << "--noheadings" << "--separator" << "|" << "--nosuffix" 
	      << "--units" << "b" << "--partial"; 
    
    ProcessProgress lvscan(arguments, "Scanning logical volumes...");
    lv_output = lvscan.programOutput();

    for(int i = 0; i < lv_output.size(); ) {
	seg_data.clear();
	lv_segments = (lv_output[i].section('|',10,10)).toInt();

	for(int x = 0 ; x < lv_segments ; x++){
	    seg_data << lv_output[i++];
	}
	LogVols.append(new LogVol(seg_data, mount_info_list));
    } 
    
    /* put pointers to the lvs in their associated vgs */
    
    lv_count = LogVols.size();
    vg_count = VolGroups.size();
    
    for(int vg = 0; vg < vg_count; vg++){
	for(int lv = 0; lv < lv_count; lv++){
	    if(LogVols[lv]->getVolumeGroupName() == VolGroups[vg]->getName()){
		VolGroups[vg]->addLogicalVolume(LogVols[lv]);
		LogVols[lv]->setVolumeGroup(VolGroups[vg]);
	    }
	}    
    }
}

void MasterList::scanLogVols(VolGroup *VolumeGroup)
{
    LogVol *logical_volume;
    QString group_name;
    QStringList lv_output;
    QStringList seg_data;
    QStringList arguments;
    int lv_segments;   //number of segments in the logical volume
    
    group_name = VolumeGroup->getName();

    mount_info_list = new MountInformationList();

    arguments << "/sbin/lvs" << "--all" << "-o" 
	      << "lv_name,vg_name,lv_attr,lv_size,origin,snap_percent,move_pv,mirror_log,copy_percent,chunksize,seg_count,stripes,stripesize,seg_size,devices,lv_kernel_major,lv_kernel_minor,lv_minor"  
	      << "--noheadings" << "--separator" << "|" << "--nosuffix" 
	      << "--units" << "b" << "--partial" << group_name; 
    
    ProcessProgress lvscan(arguments, "Scanning logical volumes...");
    lv_output = lvscan.programOutput();

    for(int i = 0; i < lv_output.size(); ) {
	seg_data.clear();
	lv_segments = (lv_output[i].section('|',10,10)).toInt();
	
	for(int x = 0 ; x < lv_segments ; x++){
	    seg_data << lv_output[i++];
	}
	logical_volume = new LogVol(seg_data, mount_info_list);
	VolumeGroup->addLogicalVolume(logical_volume);
	logical_volume->setVolumeGroup(VolumeGroup);
    } 

}

	  
/*  Once the second program 'lvs' completes we start up
    the last one 'pvs' and then get its output    */

void MasterList::scanPhysVols()
{
    QStringList pv_output;
    QStringList arguments;
    int pv_count;
    int vg_count = getVolGroupCount();
    
    arguments << "/sbin/pvs" << "--noheadings" << "--separator" << "|" << "--nosuffix" 
	      << "--units" << "b" << "--partial" << "-o" << "+pv_used,pv_uuid";
    
    ProcessProgress pvscan(arguments, "Scanning physical volumes...");
    pv_output = pvscan.programOutput();
    
    pv_count = pv_output.size();

    for(int x = 0; x < PhysVols.size(); x++) // Delete everything in the old list
	delete PhysVols[x];
    PhysVols.clear();
    
    for(int x = 0; x < pv_count; x++)
	PhysVols.append(new PhysVol(pv_output[x]));

    for(int vg = 0; vg < vg_count; vg++){
	VolGroups[vg]->clearPhysicalVolumes();
	for(int pv = 0; pv < pv_count; pv++){
	    if(PhysVols[pv]->getVolumeGroupName() == VolGroups[vg]->getName()){
		VolGroups[vg]->addPhysicalVolume(PhysVols[pv]);
	    }
	}
    }
}

void MasterList::scanPhysVols(VolGroup *VolumeGroup)
{
    PhysVol *physical_volume;
    QString group_name;
    QStringList pv_output;
    QStringList arguments;
    int pv_count;

    group_name = VolumeGroup->getName();
    
    arguments << "/sbin/pvs" << "--noheadings" << "--separator" << "|" << "--nosuffix" 
	      << "--units" << "b" << "--partial" << "-o" << "+pv_used,pv_uuid";
    
    ProcessProgress pvscan(arguments, "Scanning physical volumes...");
    pv_output = pvscan.programOutput();
    
    pv_count = pv_output.size();

    for(int x = PhysVols.size() - 1; x >= 0; x--) // Delete old list entries for this group
	if(PhysVols[x]->getVolumeGroupName() == group_name){
	    delete PhysVols[x];
	    PhysVols.removeAt(x);
	}
    
    for(int x = 0; x < pv_count; x++){
	physical_volume = new PhysVol(pv_output[x]);
	if(physical_volume->getVolumeGroupName() == group_name){
	    VolumeGroup->addPhysicalVolume(physical_volume);
	    PhysVols.append(physical_volume);
	}
	else
	    delete physical_volume;
    }
}

void MasterList::scanStorageDevices()
{
    PedDevice *dev = NULL;
    
    for(int x = 0; x < StorageDevices.size(); x++)
	delete StorageDevices[x];
    StorageDevices.clear();

    ped_device_probe_all();

    while( ( dev = ped_device_get_next(dev) ) ){
	StorageDevices.append( new StorageDevice(dev, PhysVols, mount_info_list ) );
    }
    
    delete (mount_info_list);
}

int MasterList::getVolGroupCount()
{
      return VolGroups.size();
}

int MasterList::getPhysVolCount()
{
      return PhysVols.size();
}

const QList<VolGroup *> MasterList::getVolGroups()
{
      return VolGroups;
}

const QList<PhysVol *> MasterList::getPhysVols()
{
    return PhysVols;
}

const QList<StorageDevice *> MasterList::getStorageDevices()
{
    return StorageDevices;
}

PhysVol* MasterList::getPhysVolByName(QString name)
{
    name = name.trimmed();
    for(int x = 0; x < PhysVols.size(); x++){
	if(name == PhysVols[x]->getDeviceName())
	    return PhysVols[x];
    }
    return NULL;
}

VolGroup* MasterList::getVolGroupByName(QString name)
{
    name = name.trimmed();
    for(int x = 0; x < VolGroups.size(); x++){
	if(name == VolGroups[x]->getName())
	    return VolGroups[x];
    }
    return NULL;
}

QStringList MasterList::getVolumeGroupNames()
{
    QStringList names;
    for(int x = 0; x < VolGroups.size(); x++)
	names << VolGroups[x]->getName();
    return names;
}
