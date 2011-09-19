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


#include "logvol.h"

#include <QtGui>

#include "fsdata.h"
#include "fsprobe.h"
#include "mountentry.h"
#include "mountinfo.h"
#include "mountinfolist.h"
#include "physvol.h"
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

LogVol::LogVol(lv_t lvmLV, vg_t lvmVG, VolGroup *vg, LogVol *lvParent, bool orphan):
    m_vg(vg),
    m_lv_parent(lvParent),
    m_orphan(orphan)
{
    m_snap_container   = false;
    rescan(lvmLV, lvmVG);
}

LogVol::~LogVol()
{
    QList<LogVol *> children = getChildren();

    while( children.size() )
        delete children.takeAt(0);
}

void LogVol::rescan(lv_t lvmLV, vg_t lvmVG)  // lv_t seems to change -- why?
{
    QByteArray flags;
    lvm_property_value value;
    bool was_snap_container = m_snap_container;
    m_snap_container   = false;
    m_under_conversion = false;
    m_is_origin  = false;
    m_merging    = false;
    m_mirror     = false;
    m_mirror_leg = false;
    m_mirror_log = false;
    m_snap       = false;
    m_pvmove     = false;
    m_valid      = true;
    m_virtual    = false;

    /*

      The child LogVols need to be re-used, and re-parented,
      in the case of down converted containers

      flag down converting in the following code

    */

    value = lvm_lv_get_property(lvmLV, "lv_name");
    m_lv_name = QString(value.value.string).trimmed();
    m_lv_full_name = m_vg->getName() + '/' + m_lv_name;

    processSegments(lvmLV);  // sets m_mirror according to segment property "regionsize" -- total hack!

    value = lvm_lv_get_property(lvmLV, "lv_path");
    m_lv_mapper_path = QString(value.value.string).trimmed();

    value = lvm_lv_get_property(lvmLV, "mirror_log");
    m_log = QString(value.value.string).trimmed();

    QList<lv_t> lvm_child_snapshots;
    lvm_child_snapshots.append( getLvmSnapshots(lvmVG) );

    if( ( ! lvm_child_snapshots.isEmpty() ) && m_lv_parent == NULL ){
        m_snap_container = true;
        m_seg_total = 1;
    }
    else{
        m_snap_container = false;
        value = lvm_lv_get_property(lvmLV, "seg_count");
        m_seg_total = value.value.integer;
    }

    value = lvm_lv_get_property(lvmLV, "lv_attr");
    flags.append(value.value.string);

    switch( flags[0] ){
    case 'c':
	m_type = "under conversion";
	m_mirror = true;
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
	m_type = "mirror";   // Origin status overides mirror status in the flags if this is both
	m_mirror = true;     // We split it below -- snap_containers are origins and the lv is a mirror
	break;
    case 'O':
	m_type = "origin (merging)";
        m_is_origin = true;
        m_merging = true;
	break;
    case 'o':
	m_type = "origin";
        m_is_origin = true;
	break;
    case 'p':
	m_type = "pvmove";
	m_pvmove = true;
	break;
    case 's':
	m_type = "snapshot";
	m_snap = true;
	break;
    case 'S':
        if( flags[4] != 'I' ){         // When 'S' stops getting used for Invalid and only merging - remove this
            m_type = "snapshot (merging)";
            m_snap = true;
            m_merging = true;
        }
        else{
            m_type = "snapshot";
            m_snap = true;
            m_merging = false;
        }
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
        m_valid = false;
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
    else if( m_lv_name.contains("_mimagetmp_", Qt::CaseSensitive) ){
        m_virtual = true;    // This is to get lvactionsmenu to forbid doing anything to it
        m_lv_fs = "";
    }
    else if( !m_mirror_log && !m_mirror_leg && !m_virtual){
        m_lv_fs = fsprobe_getfstype2(m_lv_mapper_path);
        m_lv_fs_label = fsprobe_getfslabel(m_lv_mapper_path);
        m_lv_fs_uuid  = fsprobe_getfsuuid(m_lv_mapper_path);
    }
    else{
        m_lv_fs = "";
        m_lv_fs_label = "";
        m_lv_fs_uuid  = "";
    }

    value = lvm_lv_get_property(lvmLV, "lv_size");
    m_size = value.value.integer;
    m_extents = m_size / m_vg->getExtentSize();

    if(m_snap || m_merging){
        value = lvm_lv_get_property(lvmLV, "origin");
        m_origin = value.value.string;
	value = lvm_lv_get_property(lvmLV, "snap_percent");
        if( value.is_valid )
            m_snap_percent = (double)value.value.integer / 1.0e+6;
        else 
            m_snap_percent = 0;
    }
    else
        m_origin = "";

    if( (m_mirror_leg || m_mirror_log) ){

        if(m_mirror_log && m_mirror_leg)
            m_type = m_type.replace("leg","log");
        else if( (m_mirror || m_virtual) && !m_mirror_log )
            m_mirror_leg = true;

    }

    value = lvm_lv_get_property(lvmLV, "copy_percent");
    if(value.is_valid)
        m_copy_percent = (double)value.value.integer / 1.0e+6;
    else
        m_copy_percent = 0;

    value = lvm_lv_get_property(lvmLV, "lv_kernel_major");
    m_major_device = value.value.integer;
    value = lvm_lv_get_property(lvmLV, "lv_kernel_minor");
    m_minor_device = value.value.integer;

    /* 
    value = lvm_lv_get_property(lvm_lv, "lv_major");

    if(value.value.integer != -1)
	m_persistant = true;
    else
	m_persistant = false;
    */

    if( m_snap_container && !was_snap_container ){
        m_uuid = QUuid::createUuid().toString();
    }
    else if( !m_snap_container ){
        value = lvm_lv_get_property(lvmLV, "lv_uuid");
        m_uuid  = value.value.string;
    }

    value = lvm_lv_get_property(lvmLV, "lv_tags");
    QString tag = value.value.string;
    m_tags = tag.split(',', QString::SkipEmptyParts);

    MountInformationList *mountInformationList = new MountInformationList();

    m_mount_info_list = mountInformationList->getMountInformation(m_lv_mapper_path);

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

    if( m_snap_container ){
        m_type = "origin";
    }
    else if( m_type.contains("origin", Qt::CaseInsensitive) && ! m_snap_container ){

        if( m_mirror )
            m_type = "mirror";
        else 
            m_type = "linear";
    }

    insertChildren(lvmLV, lvmVG);
    countLegsAndLogs();
    calculateTotalSize();
}

void LogVol::insertChildren(lv_t lvmLV, vg_t lvmVG)
{
    lv_t lvm_child;
    QByteArray  child_name;
    QStringList child_name_list;
    QList<lv_t> lvm_child_snapshots;
    lvm_child_snapshots.append( getLvmSnapshots(lvmVG) );

    while( m_lv_children.size() )
        delete m_lv_children.takeAt(0);

    if( m_snap_container ){
        for(int x = lvm_child_snapshots.size() - 1; x >= 0; x--)
            m_lv_children.append( new LogVol(lvm_child_snapshots[x], lvmVG, m_vg, this) );

        m_lv_children.append( new LogVol(lvmLV, lvmVG, m_vg, this) );
    }
    else{
        child_name_list = removePVDevices( getDevicePathAll() );

        if( m_mirror && (! m_log.isEmpty()) )
                child_name_list.append( m_log );

        for(int x = child_name_list.size() - 1; x >= 0; x--){
            child_name = child_name_list[x].toAscii();
            lvm_child = lvm_lv_from_name(lvmVG, child_name.data());
            m_lv_children.append( new LogVol(lvm_child, lvmVG, m_vg, this) );
        }
    }
}

void LogVol::countLegsAndLogs()
{
    m_mirror_count = 0;
    m_log_count = 0;
    QList<LogVol *> all_lvs_flat = getAllChildrenFlat();
    LogVol *lv;

    if(m_mirror){
        for(int x = all_lvs_flat.size() - 1; x >= 0; x--){
            lv = all_lvs_flat[x];

            if( lv->isMirrorLeg() && !lv->isMirror() && !lv->isMirrorLog() )
                m_mirror_count++;

            if( lv->isMirrorLog() && !lv->isMirror() )
                m_log_count++;
        }
    }
}

QList<lv_t> LogVol::getLvmSnapshots(vg_t lvmVG)
{
    lvm_property_value value;
    dm_list     *lv_dm_list = lvm_vg_list_lvs(lvmVG);
    lvm_lv_list *lv_list;
    QList<lv_t>  lvm_snapshots;

    if(lv_dm_list){
        dm_list_iterate_items(lv_list, lv_dm_list){ 
            
            value = lvm_lv_get_property(lv_list->lv, "origin");
            if( QString(value.value.string).trimmed() == m_lv_name )
                lvm_snapshots.append(lv_list->lv);
        }
    }

    return lvm_snapshots;
}

QStringList LogVol::removePVDevices(QStringList devices)
{
    QList<PhysVol *> pvs;
    pvs = m_vg->getPhysicalVolumes();

    for(int x = pvs.size() - 1; x >= 0; x--){
        for(int y = devices.size() - 1; y >= 0; y--){
            if( pvs[x]->getName() == devices[y] )
                devices.removeAt(y);
        }
    }

    return devices;
}

void LogVol::calculateTotalSize()
{
    m_total_size = 0;

    if( m_lv_children.size() ){
        for(int x = m_lv_children.size() - 1; x >= 0; x--)
            m_total_size += m_lv_children[x]->getTotalSize();
    }
    else{
        m_total_size = m_size;
    }
}

void LogVol::processSegments(lv_t lvmLV)
{
    Segment *segment;
    QStringList devices_and_starts, temp;
    QString raw_paths;
    lvm_property_value value;
    dm_list* lvseg_dm_list = lvm_lv_list_lvsegs(lvmLV);  
    lvm_lvseg_list *lvseg_list;
    lvseg_t lvm_lvseg;

    while( m_segments.size() )
        delete m_segments.takeAt(0);

    if(lvseg_dm_list){
        dm_list_iterate_items(lvseg_list, lvseg_dm_list){ 

            lvm_lvseg = lvseg_list->lvseg;
            value = lvm_lvseg_get_property(lvm_lvseg, "regionsize");
            if( value.is_valid ){
                if( value.value.integer )
                    m_mirror = true;
            }

            segment = new Segment();

            value = lvm_lvseg_get_property(lvm_lvseg, "stripes");
            segment->m_stripes = value.value.integer; 

            value = lvm_lvseg_get_property(lvm_lvseg, "stripesize");
            segment->m_stripe_size = value.value.integer;

            value = lvm_lvseg_get_property(lvm_lvseg, "seg_size");
            segment->m_size = value.value.integer;

            value = lvm_lvseg_get_property(lvm_lvseg, "devices");
            if( value.is_valid ){
                raw_paths = value.value.string;
            }
            if( raw_paths.size() ){
                devices_and_starts = raw_paths.split(',');
                for(int x = 0; x < devices_and_starts.size(); x++){
                    temp = devices_and_starts[x].split('(');
                    segment->m_device_path.append( temp[0] );
                    segment->m_starting_extent.append( ( temp[1].remove(')') ).toLongLong() );
                }
            }
	    m_segments.append(segment);
	}
    }
}

QList<LogVol *> LogVol::getChildren()
{
    return m_lv_children;
}

QList<LogVol *> LogVol::takeChildren()
{
    QList<LogVol *> children = m_lv_children;
    m_lv_children.clear();

    return children;
}

QList<LogVol *> LogVol::getAllChildrenFlat()
{
    QList<LogVol *> flat_list = m_lv_children;
    long child_size = m_lv_children.size();

    for(int x = 0; x < child_size; x++)
        flat_list.append( m_lv_children[x]->getAllChildrenFlat() );

    return flat_list;
}

QList<LogVol *> LogVol::getSnapshots()
{
    QList<LogVol *> snapshots;
    LogVol *top_lv = this;

    while( top_lv->getParent() != NULL)
        top_lv = top_lv->getParent();

    if( top_lv->isSnapContainer() ){
        snapshots = top_lv->getChildren();

        for(int x = snapshots.size() - 1; x >= 0; x--){  // delete the 'real' lv leaving the snaps
            if( m_lv_name == snapshots[x]->getName() )
                snapshots.removeAt(x);
        }
    }

    return snapshots;
}

LogVol *LogVol::getParent()
{
    return m_lv_parent;
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

    devices.sort();
    devices.removeDuplicates();

    return devices;
}

QStringList LogVol::getDevicePathAllFlat()
{
    QStringList devices;
    QList<LogVol *> children;

    if( m_snap_container || m_mirror ){
        children = getChildren();

        for(int x = children.size() - 1; x >= 0; x--)
            devices.append( children[x]->getDevicePathAllFlat() );

        devices.sort();
        devices.removeDuplicates();

        return devices;
    }
    else
        return getDevicePathAll();
}

VolGroup* LogVol::getVolumeGroup()
{
    return m_vg;
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
    return m_lv_mapper_path;
}

long long LogVol::getSpaceOnPhysicalVolume(QString physicalVolume)
{
    long long space_used = 0;
    for(int x = getSegmentCount() - 1; x >= 0; x--){
	for(int y = m_segments[x]->m_device_path.size() - 1; y >= 0; y--){
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

long long LogVol::getTotalSize()
{
    return m_total_size;
}

QString LogVol::getFilesystem()
{
  return m_lv_fs;
}

QString LogVol::getFilesystemLabel()
{
  return m_lv_fs_label;
}

QString LogVol::getFilesystemUuid()
{
  return m_lv_fs_uuid;
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

int LogVol::getSnapshotCount()
{
    return getSnapshots().size(); 
}

bool LogVol::isMerging()
{
    return m_merging;
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

bool LogVol::isSnapContainer()
{
    return m_snap_container;
}

bool LogVol::isPvmove()
{
    return m_pvmove;
}

bool LogVol::isOrigin()
{
    return m_is_origin;
}

bool LogVol::isOrphan()
{
    return m_orphan;
}

bool LogVol::isFixed()
{
    return m_fixed;
}

bool LogVol::isValid()
{
    return m_valid;
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
   list of objects containing mount point and relevant
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
    if( m_snap || m_merging )
        return m_snap_percent;
    else
        return 0.0;
}

double LogVol::getCopyPercent()
{
    if( m_pvmove )
        return m_copy_percent;
    else
        return 0.0;
}

QString LogVol::getUuid()
{
  return m_uuid;
}
