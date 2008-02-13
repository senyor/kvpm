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
#include "logvol.h"
#include "mountentry.h"
#include "mountinfo.h"
#include "processprogress.h"
#include "storagedevice.h"


/* the logical volume data is in a list because lvs outputs one
   line for each segment  */

LogVol::LogVol(QStringList lvdata_list, MountInformationList *mount_list)
{
    QString lvdata = lvdata_list[0];
    lvdata = lvdata.trimmed();
    QByteArray flags;
    seg_total = lvdata_list.size();
    lv_name = lvdata.section('|',0,0);
    lv_name = lv_name.trimmed();
    vg_name = lvdata.section('|',1,1);
    full_name = vg_name + "/" + lv_name;
    QString attr = lvdata.section('|',2,2);
    flags.append(attr);
    switch( flags[0] ){
    case 'M':
	type = "mirror";
	break;
    case 'm':
	type = "mirror";
	break;
    case 'o':
	type = "origin";
	break;
    case 'p':
	type = "pvmove";
	break;
    case 's':
	type = "valid snap";
	break;
    case 'S':
	type = "invalid snap";
	break;
    case 'v':
	type = "virtual";
	break;
    default:
	type = "standard";
    break;
    }

    snap    = type.contains("snap", Qt::CaseInsensitive);
    pvmove = type.contains("pvmove", Qt::CaseInsensitive);
    
    switch(flags[1]){
    case 'w':
	writable = TRUE;
	break;
    default:
	writable = FALSE;
    }
    alloc_locked = FALSE;
    switch( flags[2] ){
    case 'C':
	alloc_locked = TRUE;
    case 'c':
	policy = "Contiguous";
	break;
    case 'L':
	alloc_locked = TRUE;
    case 'l':
	policy = "Cling";
	break;
    case 'N':
	alloc_locked = TRUE;
    case 'n':
	policy = "Normal";
	break;
    case 'A':
	alloc_locked = TRUE;
    case 'a':
	policy = "Anywhere";
	break;
    case 'I':
	alloc_locked = TRUE;
    case 'i':
	policy = "Inherited";
	break;
    default:
	policy = "Other";
	break;
    }
    if(alloc_locked)
	policy.append(" (locked)");
    switch(flags[3]){
    case 'm':
	fixed = TRUE;
	break;
    default:
	fixed = FALSE;
    }
    switch(flags[4]){
    case '-':
	state = "Unavailable";
	break;
    case 'a':
	state = "Active";
	break;
    case 's':
	state = "Suspended";
	break;
    case 'I':
	state = "Invalid";
	break;
    case 'S':
	state = "Suspended";
	break;
    case 'd':
	state = "No table";
	break;
    case 'i':
	state = "Inactive table";
	break;
    default:
	state = "Other";
    }
    switch(flags[5]){
    case 'o':
	open = TRUE;
	break;
    default:
	open = FALSE;
    }
    size = (lvdata.section('|',3,3)).toLongLong();
    snap_origin = lvdata.section('|',4,4);
    snap_percent = (lvdata.section('|',5,5)).toDouble();
    copy_percent = (lvdata.section('|',8,8)).toDouble();
    major_device = (lvdata.section('|',15,15)).toInt();
    minor_device = (lvdata.section('|',16,16)).toInt();
    if( (lvdata.section('|',17,17)).toInt() != -1)
	persistant = TRUE;
    else
	persistant = FALSE;
    lv_fs = fsprobe_getfstype2( "/dev/mapper/" + vg_name + "-" + lv_name );

    
    mount_info_list = mount_list->getMountInformation( "/dev/mapper/" + vg_name + "-" + lv_name );

/* To Do: get all the rest of the mount info, not just mount points */

    for(int x = 0; x < mount_info_list.size(); x++){
	mount_points <<  mount_info_list[x]->getMountPoint();
	delete mount_info_list[x];
    }
    mount_info_list.clear();
    
    
    if( mount_points.size() )
	mounted = TRUE;
    else
	mounted = FALSE;
    
    for(int x = 0; x < seg_total ; x++)
	segments.append(processSegments(lvdata_list[x]));
}

Segment* LogVol::processSegments(QString segment_data)
{
    Segment *segment = new Segment();
    
    QStringList devices_and_starts, temp;
    QString raw_paths;
    
    segment->stripes = (segment_data.section('|',11,11)).toInt();
    segment->stripe_size = (segment_data.section('|',12,12)).toInt();
    segment->size = (segment_data.section('|',13,13)).toLongLong();

    raw_paths = (segment_data.section('|',14,14)).trimmed();
    devices_and_starts = raw_paths.split(",");
    for(int x = 0; x < devices_and_starts.size(); x++){
	temp = devices_and_starts[x].split("(");
	segment->device_path.append(temp[0]);
	segment->starting_extent.append((temp[1].remove(")")).toLongLong());
    }
    return segment;
}

int LogVol::getSegmentCount()
{
    return seg_total;
}

int LogVol::getSegmentStripes(int segment)
{
    return segments[segment]->stripes;
}

int LogVol::getSegmentStripeSize(int segment)
{
    return segments[segment]->stripe_size;
}

long long LogVol::getSegmentSize(int segment)
{
    return segments[segment]->size;
}

long long LogVol::getSegmentExtents(int segment)
{
    return (segments[segment]->size / group->getExtentSize());
}

QList<long long> LogVol::getSegmentStartingExtent(int segment)
{
    return segments[segment]->starting_extent;
}

QStringList LogVol::getDevicePath(int segment)
{
    return segments[segment]->device_path;
}

QStringList LogVol::getDevicePathAll()
{
    QStringList devices;
    
    for (int seg = 0; seg < seg_total; seg++)
	devices << segments[seg]->device_path;
    return devices;
}

void LogVol::setVolumeGroup(VolGroup *VolumeGroup)
{
    group = VolumeGroup;
    extents = size / group->getExtentSize();
}

VolGroup* LogVol::getVolumeGroup()
{
    return group;
}

QString LogVol::getVolumeGroupName()
{
    return vg_name;
}

QString LogVol::getName()
{
    return lv_name;
}

QString LogVol::getFullName()
{
    return full_name;
}

const QString LogVol::getMapperPath()
{
    QString path;

    path = "/dev/mapper/" + vg_name + "-" + lv_name;
    
    return path;
}

long long LogVol::getSpaceOnPhysicalVolume(QString PhysicalVolume)
{
    long long space_used = 0;
    for(int x = 0; x < getSegmentCount(); x++){
	for(int y = 0; y < segments[x]->device_path.size(); y++){
	    if(PhysicalVolume == (segments[x])->device_path[y]){
		space_used += (segments[x]->size) / (segments[x]->stripes) ;
	    }
	}
    }
    return space_used;
}


long long LogVol::getExtents()
{
  return extents;
}

long long LogVol::getSize()
{
  return size;
}

QString LogVol::getFS()
{
  return lv_fs;
}

int LogVol::getMinorDevice()
{
    return minor_device;
}

int LogVol::getMajorDevice()
{
    return major_device;
}

bool LogVol::isMounted()
{
    return mounted;
}

bool LogVol::isPersistant()
{
    return persistant;
}

bool LogVol::isOpen()
{
    return open;
}

bool LogVol::isLocked()
{
    return alloc_locked;
}

bool LogVol::isWritable()
{
    return writable;
}

bool LogVol::isSnap()
{
    return snap;
}

bool LogVol::isPvmove()
{
    return pvmove;
}

bool LogVol::isOrigin()
{
    return type.contains("origin", Qt::CaseInsensitive);
}

bool LogVol::isFixed()
{
    return fixed;
}

QString LogVol::getPolicy()
{
    return policy;
}

QString LogVol::getState()
{
    return state;
}

QString LogVol::getType()
{
    return type;
}

QString LogVol::getOrigin()
{
    return  snap_origin;
}

QStringList LogVol::getMountPoints()
{
    return mount_points;
}

double LogVol::getSnapPercent()
{
    return snap_percent;
}

double LogVol::getCopyPercent()
{
    return copy_percent;
}

