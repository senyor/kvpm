/*
 *
 * 
 * Copyright (C) 2008, 2010 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
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
#include "storagedevice.h"
#include "volgroup.h"


/* Some information about a logical volume pertains to the entire volume
   while other information only applies to a segement in the volume. The
   volume keeps a list of Segment structures for segment information */

struct Segment
{
    int m_stripes;               // number of stripes in segment
    int m_stripe_size;    
    long long m_size;            // segment size (bytes)
    QStringList m_device_path;   // full path of physical volume
    QList<long long> m_starting_extent;   // first extent on physical volume 
                                          // for this segment
};

/* the logical volume data is in a list because lvs outputs one
   line for each segment  */

LogVol::LogVol(QStringList lvDataList, MountInformationList *mountInformationList)
{
    QString lvdata = lvDataList[0].trimmed();
    QByteArray flags;

    m_seg_total = lvDataList.size();
    m_lv_name   = lvdata.section('|',0,0);
    m_lv_name   = m_lv_name.trimmed();

    m_vg_name   = lvdata.section('|',1,1);
    m_lv_full_name = m_vg_name + "/" + m_lv_name;

    QString attr = lvdata.section('|',2,2);
    flags.append(attr);
	
    m_under_conversion = false;
    m_mirror     = false;
    m_mirror_leg = false;
    m_mirror_log = false;
    m_snap       = false;
    m_pvmove     = false;
    m_virtual    = false;
    
    switch( flags[0] ){
    case 'c':
	m_type = "under conversion";
	m_under_conversion = true;
	break;
    case 'I':
	m_type = "un-synced mirror leg";
	m_mirror_leg = true;
	break;
    case 'i':
	m_type = "synced mirror leg";
	m_mirror_leg = true;
	break;
    case 'L':
    case 'l':
	m_type = "mirror log";
	m_mirror_log = true;
	break;
    case 'M':
	m_type = "mirror";
	m_mirror = true;
	break;
    case 'm':
	m_type = "mirror";
	m_mirror = true;
	break;
    case 'o':
	m_type = "origin";
	break;
    case 'p':
	m_type = "pvmove";
	m_pvmove = true;
	break;
    case 's':
	m_type = "valid snap";
	m_snap = true;
	break;
    case 'S':
	m_type = "invalid snap";
	m_snap = true;
	break;
    case 'v':
	m_type = "virtual";
	m_virtual = true;
	break;
    default:
	m_type = "standard";
    break;
    }

    switch(flags[1]){
    case 'w':
	m_writable = true;
	break;
    default:
	m_writable = false;
    }

    m_alloc_locked = false;

    switch( flags[2] ){
    case 'C':
	m_alloc_locked = true;
    case 'c':
	m_policy = "Contiguous";
	break;
    case 'L':
	m_alloc_locked = true;
    case 'l':
	m_policy = "Cling";
	break;
    case 'N':
	m_alloc_locked = true;
    case 'n':
	m_policy = "Normal";
	break;
    case 'A':
	m_alloc_locked = true;
    case 'a':
	m_policy = "Anywhere";
	break;
    case 'I':
	m_alloc_locked = true;
    case 'i':
	m_policy = "Inherited";
	break;
    default:
	m_policy = "Other";
	break;
    }

    if(m_alloc_locked)
	m_policy.append(" (locked)");

    switch(flags[3]){
    case 'm':
	m_fixed = true;
	break;
    default:
	m_fixed = false;
    }

    m_active = false;
    
    switch(flags[4]){
    case '-':
	m_state = "Unavailable";
	break;
    case 'a':
	m_state = "Active";
	m_active = true;
	break;
    case 's':
	m_state = "Suspended";
	break;
    case 'I':
	m_state = "Invalid";
	break;
    case 'S':
	m_state = "Suspended";
	break;
    case 'd':
	m_state = "No table";
	break;
    case 'i':
	m_state = "Inactive table";
	break;
    default:
	m_state = "Other";
    }

    switch(flags[5]){
    case 'o':
	m_open = true;
	break;
    default:
	m_open = false;
    }

    m_size         = (lvdata.section('|',3,3)).toLongLong();

    if(m_snap){
	m_origin       =  lvdata.section('|',4,4);
	m_snap_percent = (lvdata.section('|',5,5)).toDouble();
    }
    else if(m_mirror_leg){
	m_origin = m_lv_name;
	m_origin.remove(0,1);
	m_origin.truncate( m_origin.indexOf("_mimage_") );
	m_snap_percent = 0.0;
    }
    else if(m_mirror_log){
	m_origin = m_lv_name;
	m_origin.remove(0,1);
	m_origin.truncate( m_origin.indexOf("_mlog]") );
	m_snap_percent = 0.0;
    }
    else if( m_mirror || m_virtual ){
	if( m_lv_name.contains("_mimagetmp_") )
	{
	    m_origin = m_lv_name;
	    m_origin.remove(0,1);
	    m_origin.truncate( m_origin.indexOf("_mimagetmp_") );
	    m_snap_percent = 0.0;
	}
	else{
	    m_origin = "";
	    m_snap_percent = 0.0;
	}
    }
    else{
	m_origin = "";
	m_snap_percent = 0.0;
    }
    

    m_copy_percent = (lvdata.section('|',8,8)).toDouble();
    m_major_device = (lvdata.section('|',15,15)).toInt();
    m_minor_device = (lvdata.section('|',16,16)).toInt();

    if( (lvdata.section('|',17,17)).toInt() != -1)
	m_persistant = true;
    else
	m_persistant = false;

    m_uuid  = lvdata.section('|',18,18);
    m_lvm_format = lvdata.section('|',19,19);
    m_vg_attr = lvdata.section('|',20,20);

    if( m_lv_name.contains("_mlog", Qt::CaseSensitive) )
        m_lv_fs = "mirror log";
    else
        m_lv_fs = fsprobe_getfstype2( "/dev/mapper/" + m_vg_name + "-" + m_lv_name );

    m_mount_info_list = mountInformationList->getMountInformation( "/dev/mapper/" + 
							 m_vg_name + "-" + m_lv_name );

/* To Do: get all the rest of the mount info, not just mount points */

    for(int x = 0; x < m_mount_info_list.size(); x++){
        m_mount_points.append( m_mount_info_list[x]->getMountPoint() );
	m_mount_position.append( m_mount_info_list[x]->getMountPosition() );
	delete m_mount_info_list[x];
    }
    m_mount_info_list.clear();

    m_mounted = !m_mount_points.isEmpty();
    
    for(int x = 0; x < m_seg_total ; x++)
	m_segments.append(processSegments(lvDataList[x]));
    
}

Segment* LogVol::processSegments(QString segmentData)
{
    Segment *segment = new Segment();
    
    QStringList devices_and_starts, temp;
    QString raw_paths;
    
    segment->m_stripes     = (segmentData.section('|',11,11)).toInt();
    segment->m_stripe_size = (segmentData.section('|',12,12)).toInt();
    segment->m_size        = (segmentData.section('|',13,13)).toLongLong();
    
    raw_paths = (segmentData.section('|',14,14)).trimmed();

    if( raw_paths.size() ){
        qDebug() << "Starts:  " << raw_paths;
	devices_and_starts = raw_paths.split(",");

	for(int x = 0; x < devices_and_starts.size(); x++){
	    temp = devices_and_starts[x].split("(");
	    segment->m_device_path.append(temp[0]);
	    segment->m_starting_extent.append((temp[1].remove(")")).toLongLong());
	}
    }
    
    return segment;
}

int LogVol::getSegmentCount()
{
    return m_seg_total;
}

int LogVol::getSegmentStripes(int segment)
{
    return m_segments[segment]->m_stripes;
}

int LogVol::getSegmentStripeSize(int segment)
{
    return m_segments[segment]->m_stripe_size;
}

long long LogVol::getSegmentSize(int segment)
{
    return m_segments[segment]->m_size;
}

long long LogVol::getSegmentExtents(int segment)
{
    return (m_segments[segment]->m_size / m_group->getExtentSize());
}

QList<long long> LogVol::getSegmentStartingExtent(int segment)
{
    return m_segments[segment]->m_starting_extent;
}

QStringList LogVol::getDevicePath(int segment)
{
    return m_segments[segment]->m_device_path;
}

QStringList LogVol::getDevicePathAll()
{
    QStringList devices;
    
    for (int seg = 0; seg < m_seg_total; seg++)
	devices << m_segments[seg]->m_device_path;

// remove repeated physical volumes

    devices.sort();

    for(int x = devices.size() - 1; x > 0; x--){
	if(devices[x] == devices[x - 1])
	    devices.removeAt(x);
    }
    
    return devices;
}

void LogVol::setVolumeGroup(VolGroup *volumeGroup)
{
    m_group = volumeGroup;
    m_extents = m_size / m_group->getExtentSize();
}

VolGroup* LogVol::getVolumeGroup()
{
    return m_group;
}

QString LogVol::getVolumeGroupName()
{
    return m_vg_name;
}

QString LogVol::getName()
{
    return m_lv_name;
}

QString LogVol::getFullName()
{
    return m_lv_full_name;
}

QString LogVol::getMapperPath()
{
    QString path;

    path = "/dev/mapper/" + m_vg_name + "-" + m_lv_name;
    
    return path;
}

long long LogVol::getSpaceOnPhysicalVolume(QString physicalVolume)
{
    long long space_used = 0;
    for(int x = 0; x < getSegmentCount(); x++){
	for(int y = 0; y < m_segments[x]->m_device_path.size(); y++){
	    if(physicalVolume == (m_segments[x])->m_device_path[y]){
		space_used += (m_segments[x]->m_size) / (m_segments[x]->m_stripes) ;
	    }
	}
    }
    return space_used;
}


long long LogVol::getExtents()
{
  return m_extents;
}

long long LogVol::getSize()
{
  return m_size;
}

QString LogVol::getFilesystem()
{
  return m_lv_fs;
}

int LogVol::getMinorDevice()
{
    return m_minor_device;
}

int LogVol::getMajorDevice()
{
    return m_major_device;
}

bool LogVol::isMounted()
{
    return m_mounted;
}

bool LogVol::isActive()
{
    return m_active;
}

bool LogVol::isMirror()
{
    return m_mirror;
}

bool LogVol::isMirrorLeg()
{
    return m_mirror_leg;
}

bool LogVol::isMirrorLog()
{
    return m_mirror_log;
}

bool LogVol::isPersistant()
{
    return m_persistant;
}

bool LogVol::isOpen()
{
    return m_open;
}

bool LogVol::isLocked()
{
    return m_alloc_locked;
}

bool LogVol::isUnderConversion()
{
    return m_under_conversion;
}

bool LogVol::isWritable()
{
    return m_writable;
}

bool LogVol::isVirtual()
{
    return m_virtual;
}

bool LogVol::isSnap()
{
    return m_snap;
}

bool LogVol::isPvmove()
{
    return m_pvmove;
}

bool LogVol::isOrigin()
{
    return m_type.contains("origin", Qt::CaseInsensitive);
}

bool LogVol::isFixed()
{
    return m_fixed;
}

QString LogVol::getPolicy()
{
    return m_policy;
}

QString LogVol::getState()
{
    return m_state;
}

QString LogVol::getType()
{
    return m_type;
}

QString LogVol::getLVMFormat()
{
    return m_lvm_format;
}

QString LogVol::getVGAttr()
{
    return m_vg_attr;
}

QString LogVol::getOrigin()
{
    return  m_origin;
}

/* TO DO: Merge these next two lists in a single
   list of objects containing mount point and relevent
   mount position. See "mountinfo.h" for more on mount 
   position. */

QStringList LogVol::getMountPoints()
{
    return m_mount_points;
}

QList<int> LogVol::getMountPosition()
{
    return m_mount_position;
}

double LogVol::getSnapPercent()
{
    return m_snap_percent;
}

double LogVol::getCopyPercent()
{
    return m_copy_percent;
}

QString LogVol::getUuid()
{
  return m_uuid;
}
