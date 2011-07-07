/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
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

#include "fsdata.h"
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

LogVol::LogVol(lv_t lvm_lv, VolGroup *vg)
{
    m_vg = vg;
    rescan(lvm_lv);
}

void LogVol::rescan(lv_t lvm_lv)
{
    QByteArray flags;
    lvm_property_value value;

    m_log_count = 0;

    value = lvm_lv_get_property(lvm_lv, "seg_count");
    m_seg_total = value.value.integer;

    value = lvm_lv_get_property(lvm_lv, "lv_name");
    m_lv_name   = QString(value.value.string).trimmed();

    m_vg_name   = m_vg->getName();
    m_lv_full_name = m_vg_name + "/" + m_lv_name;

    value = lvm_lv_get_property(lvm_lv, "lv_attr");
    flags.append(value.value.string);

    m_under_conversion = false;
    m_mirror     = false;
    m_mirror_leg = false;
    m_mirror_log = false;
    m_snap       = false;
    m_pvmove     = false;
    m_virtual    = false;
    m_orphan     = false;

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
    case 'M':                // mirror logs can be mirrors themselves -- see below
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
	m_type = "linear";
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

    if( m_lv_name.contains("_mlog", Qt::CaseSensitive) ){
        m_mirror_log = true;    // this needs to be here in case it is a mirrored mirror log
        m_lv_fs = "";
    }
    else
        m_lv_fs = fsprobe_getfstype2( "/dev/" + m_vg_name + "/" + m_lv_name );
 
    value = lvm_lv_get_property(lvm_lv, "lv_size");
    m_size = value.value.integer;
    m_extents = m_size / m_vg->getExtentSize();

    if(m_snap){
        value = lvm_lv_get_property(lvm_lv, "origin");
        m_origin = value.value.string;
	value = lvm_lv_get_property(lvm_lv, "snap_percent");
        if( value.is_valid )
            m_snap_percent = (double)value.value.integer / 1.0e+6;
        else 
            m_snap_percent = 0;
    }
    else if(m_mirror_leg && !m_mirror_log){
        m_origin = m_lv_name;
        m_origin.truncate( m_origin.indexOf("_mimage_") );
    }
    else if(m_mirror_log && !m_mirror_leg){
        m_origin = m_lv_name;
        m_origin.truncate( m_origin.indexOf("_mlog") );
    }
    else if(m_mirror_log && m_mirror_leg){
        m_origin = m_lv_name;
        m_origin.truncate( m_origin.indexOf("_mimage_") );
        m_type = m_type.replace("leg","log");
    }
    else if( m_mirror || m_virtual ){
	if( m_lv_name.contains("_mimagetmp_") ){
            m_origin = m_lv_name;
	    m_origin.remove(0,1);
	    m_origin.truncate( m_origin.indexOf("_mimagetmp_") );
	}
	else
	    m_origin = "";
    }
    else
        m_origin = "";

    value = lvm_lv_get_property(lvm_lv, "copy_percent");
    if(value.is_valid)
        m_copy_percent = (double)value.value.integer / 1.0e+6;
    else
        m_copy_percent = 0;

    value = lvm_lv_get_property(lvm_lv, "lv_kernel_major");
    m_major_device = value.value.integer;
    value = lvm_lv_get_property(lvm_lv, "lv_kernel_minor");
    m_minor_device = value.value.integer;

    /* 
    value = lvm_lv_get_property(lvm_lv, "lv_major");

    if(value.value.integer != -1)
	m_persistant = true;
    else
	m_persistant = false;
    */

    value = lvm_lv_get_property(lvm_lv, "lv_uuid");
    m_uuid  = value.value.string;

    value = lvm_lv_get_property(lvm_lv, "lv_tags");
    QString tag = value.value.string;
    m_tags = tag.split(',', QString::SkipEmptyParts);

    MountInformationList *mountInformationList = new MountInformationList();
    m_mount_info_list = mountInformationList->getMountInformation( "/dev/" + m_vg_name + "/" + m_lv_name );

/* To Do: get all the rest of the mount info, not just mount points */

    m_mount_points.clear();
    m_mount_position.clear();

    for(int x = 0; x < m_mount_info_list.size(); x++){
        m_mount_points.append( m_mount_info_list[x]->getMountPoint() );
	m_mount_position.append( m_mount_info_list[x]->getMountPosition() );
	delete m_mount_info_list[x];
    }
    m_mount_info_list.clear();
    delete mountInformationList;

    m_mounted = !m_mount_points.isEmpty();

    if(m_mounted){
        FSData *fs_data = get_fs_data(m_mount_points[0]);
        m_block_size = fs_data->block_size;
        m_fs_size = (long long)fs_data->size * m_block_size;
        m_fs_used = (long long)fs_data->used * m_block_size;
        delete fs_data;
    }
    else{
        m_block_size = -1;
        m_fs_size = -1;
        m_fs_used = -1;
    }

    for(int x = 0; x < m_segments.size(); x++)
        delete m_segments[x];
    m_segments.clear();

    dm_list* lvseg_dm_list = lvm_lv_list_lvsegs(lvm_lv);  
    lvm_lvseg_list *lvseg_list;

    if(lvseg_dm_list){
        dm_list_iterate_items(lvseg_list, lvseg_dm_list){ 
	    m_segments.append(processSegments(lvseg_list->lvseg));
	}
    }

    m_mirror_count = 0;
    QStringList pvs = getDevicePathAll();

    for(int x =0; x < pvs.size(); x++){
        if( pvs[x].contains("_mimage_") ){
            m_mirror = true;
            if( !pvs[x].contains("_mlog_") )
                m_mirror_count++;
        }
    }
}

Segment* LogVol::processSegments(lvseg_t lvm_lvseg)
{
    Segment *segment = new Segment();
    QStringList devices_and_starts, temp;
    QString raw_paths;
    lvm_property_value value;

    value = lvm_lvseg_get_property(lvm_lvseg, "stripes");
    segment->m_stripes = value.value.integer; 

    value = lvm_lvseg_get_property(lvm_lvseg, "stripesize");
    segment->m_stripe_size = value.value.integer;

    value = lvm_lvseg_get_property(lvm_lvseg, "seg_size");
    segment->m_size = value.value.integer;

    value = lvm_lvseg_get_property(lvm_lvseg, "devices");
    if( value.is_valid )
        raw_paths = value.value.string;

    if(raw_paths == "" && m_virtual)
        m_orphan = true;

    if( raw_paths.size() ){
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

int LogVol::getSegmentStripes(int segment)  // number of mirrors if this is a mirror
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
    return (m_segments[segment]->m_size / m_vg->getExtentSize());
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

VolGroup* LogVol::getVolumeGroup()
{
    return m_vg;
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

    path = "/dev/" + m_vg_name + "/" + m_lv_name;
    
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

long long LogVol::getFilesystemSize()
{
    return m_fs_size;
}

long long LogVol::getFilesystemUsed()
{
    return m_fs_used;
}

long long LogVol::getFilesystemBlockSize()
{
    return m_block_size;
}

unsigned long LogVol::getMinorDevice()
{
    return m_minor_device;
}

unsigned long LogVol::getMajorDevice()
{
    return m_major_device;
}

int LogVol::getLogCount()
{
    return m_log_count;
}

int LogVol::getMirrorCount()
{
    return m_mirror_count;
}

void LogVol::setLogCount(int logCount)
{
    m_log_count = logCount;
    return;
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

bool LogVol::isOrphan()
{
    QList<LogVol *> lvs = m_vg->getLogicalVolumes();
    bool has_invalid_origin = false;

    if( m_mirror_log || m_mirror_leg ){ // look for legs and logs of lvs that are not mirrors!
        has_invalid_origin = true;
        for(int x = 0; x < lvs.size(); x++){
            if( lvs[x]->getName() == m_origin && lvs[x]->isMirror() )
                has_invalid_origin = false;  
        } 
    }

    return ( m_orphan || has_invalid_origin );
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

QStringList LogVol::getTags()
{
    return m_tags;
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
