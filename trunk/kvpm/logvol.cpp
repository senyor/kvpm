/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013, 2014, 2016 Benjamin Scott   <benscott@nwlink.com>
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <QFileInfo>
#include <QRegExp>
#include <QUuid>


#include "fsdata.h"
#include "fsprobe.h"
#include "masterlist.h"
#include "physvol.h"
#include "storagedevice.h"
#include "volgroup.h"


/* Some information about a logical volume pertains to the entire volume
   while other information only applies to a segement in the volume. The
   volume keeps a list of Segment structures for segment information */


struct Segment {
    int stripes;                 // number of stripes in segment
    int stripe_size;
    long long size;              // segment size (bytes)
    QString type;                // linear, raid etc.
    QStringList device_path;     // full path of physical volume
    QList<long long> starting_extent;   // first extent on physical volume
                                            // for this segment
    QString discards;            // May be: ignore, nopassdown, passdown
    long long chunk_size;        // Thin pool chunk size
};


LogVol::LogVol(lv_t lvmLv, vg_t lvmVg, const VolGroup *const vg, LogVol *const lvParent, 
               MountTables *const tables, const bool orphan) :
    m_vg(vg),
    m_tables(tables),
    m_lv_parent(lvParent),
    m_orphan(orphan)
{
    m_snap_container = false;
    rescan(lvmLv, lvmVg);
}

LogVol::~LogVol()
{
    for (auto ptr : m_segments)
        delete ptr;
}

void LogVol::rescan(lv_t lvmLv, vg_t lvmVg)
{
    bool was_snap_container = m_snap_container;
    m_snap_container   = false;
    m_under_conversion = false;
    m_is_origin   = false;       // traditional snap origin - not thin snap origin
    m_merging     = false;
    m_metadata    = false;
    m_raid_metadata  = false;
    m_thin_metadata  = false;
    m_raidmirror     = false;
    m_raidmirror_leg = false;
    m_lvmmirror      = false;
    m_lvmmirror_leg  = false;
    m_lvmmirror_log  = false;
    m_raid        = false;
    m_raid_image  = false;
    m_cow_snap    = false;
    m_temp        = false;
    m_thin        = false;
    m_thin_data   = false;
    m_thin_pool   = false;
    m_pvmove      = false;
    m_valid       = true;
    m_synced      = true;
    m_virtual     = false;
    m_copy_percent = 0;
    m_data_percent = 0;
    m_snap_percent = 0;

    m_lv_name = QString(lvm_lv_get_property(lvmLv, "lv_name").value.string).trimmed();
    m_lv_full_name = m_vg->getName() + '/' + m_lv_name;
    //    m_lv_mapper_path = QString(lvm_lv_get_property(lvmLv, "lv_path").value.string).trimmed(); // borked!
    m_lv_mapper_path = QString("/dev/").append(m_lv_full_name); // workaround
    m_log = QString(lvm_lv_get_property(lvmLv, "mirror_log").value.string).trimmed();
    m_pool = QString(lvm_lv_get_property(lvmLv, "pool_lv").value.string).trimmed();
    m_tags = QString(lvm_lv_get_property(lvmLv, "lv_tags").value.string).split(',', QString::SkipEmptyParts);
    m_size = lvm_lv_get_property(lvmLv, "lv_size").value.integer;
    m_extents = m_size / m_vg->getExtentSize();

    if (lvm_lv_get_property(lvmLv, "lv_kernel_major").is_valid && lvm_lv_get_property(lvmLv, "lv_kernel_minor").is_valid) {
        m_major_device = lvm_lv_get_property(lvmLv, "lv_kernel_major").value.integer;
        m_minor_device = lvm_lv_get_property(lvmLv, "lv_kernel_minor").value.integer;
    } else { // some versions of lvm2app library don't return valid numbers
        getDeviceNumbers(m_major_device, m_minor_device);
    }

    m_persistent = (-1 != (static_cast<int64_t>(lvm_lv_get_property(lvmLv, "lv_major").value.integer)));

    QByteArray flags(lvm_lv_get_property(lvmLv, "lv_attr").value.string);
    m_writable = (flags[1] == 'w');
    m_fixed    = (flags[3] == 'm'); 
    m_open     = (flags[5] == 'o');
    m_zero     = (flags[7] == 'z');
    m_partial  = (flags[8] == 'p');

    processSegments(lvmLv, flags);
    setSnapContainer(lvmVg, lvmLv);

    QString additional_state;

    switch (flags[0]) {
    case 'c':
        m_type = "under conversion";
        m_lvmmirror = true;
        m_under_conversion = true;
        break;
    case 'e':
        m_type = "metadata";
        m_metadata = true;

        if (flags[6] == 'r'){
            m_type = m_segments[0]->type;
            m_raid = true;
        }

        break;
    case 'I':
    case 'i':
        if (flags[6] == 'r'){
            if (m_lv_parent) {
                if (m_lv_parent->isRaid() && m_lv_parent->getRaidType() == 1) {
                    m_type = "mirror leg";
                    m_raidmirror_leg = true;
                } else {
                    m_type = "raid image";
                }
            } else {
                m_type = "raid image";
            }
            
            m_raid_image = true;
        } else {
            m_type = "mirror leg";
            m_lvmmirror_leg = true;
        }
        
        if(flags[0] == 'I') {
            additional_state = "un-synced";
            m_synced = false;
        } else {
            additional_state = "synced";
            m_synced = true;
        }
        
        break;
    case 'L':
    case 'l':
        m_type = "mirror log";
        m_lvmmirror_log = true;
        break;
    case 'M':                // mirror logs can be mirrors themselves -- see below
        m_type = "lvm mirror";
        m_lvmmirror = true;
        break;
    case 'm':
        m_type = "lvm mirror";  // Origin status overides mirror status in the flags if this is both
        m_lvmmirror = true;     // We split it below -- snap_containers are origins and the lv is a mirror
        break;
    case 'O':
        m_type = "origin";
        additional_state = "merging";
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
    case 'r':
        m_type = m_segments[0]->type;
        m_raid = true;
        break;
    case 'R':
        m_type = m_segments[0]->type;
        m_raid = true;
        break;
    case 's':
        m_type = "snapshot";
        m_cow_snap = true;
        break;
    case 'S':
        m_type = "snapshot";
        additional_state = "merging";
        m_cow_snap = true;
        m_merging = true;
        break;
    case 't':
        m_type = "thin pool";
        m_thin_pool = true;
        break;
    case 'T':
        m_type = "thin data";
        m_thin_data = true;
        break;
    case 'v':
        m_type = "virtual";
        m_virtual = true;
        break;
    case 'V':
        m_type = "thin volume";     // For Non-thin snaps origin status overrides this in the flags -- we add it back below
        m_thin = true;
        break;
    default:
        m_type = "linear";
        break;
    }

    if (m_lv_name.contains("_rmeta_")) {
        m_raid_metadata = true;
        m_metadata = true;
    }

    if (m_lv_name.contains("_tmeta_") || m_lv_name.endsWith("_tmeta")) {
        m_thin_metadata = true;
        m_metadata = true;
    }

    if ((flags[6] == 't') && m_is_origin)
        m_thin = true;

    if (m_lv_name.contains("_tdata_") || m_lv_name.endsWith("_tdata"))
        m_thin_data = true;

    lvm_property_value value;
    if (flags[6] == 't') {
        value = lvm_lv_get_property(lvmLv, "origin");
        if (value.is_valid && (QString(value.value.string) != "")) {
            m_origin = value.value.string;
            m_cow_snap  = false;
            m_thin_snap = true;
            m_thin = true;
            m_type = "thin snapshot";
        }
    }

    setPolicy(flags[2]);

    m_active = false;

    switch (flags[4]) {
    case '-':
        m_state = "inactive";
        break;
    case 'a':
        m_state = "active";
        m_active = true;
        break;
    case 's':
        m_state = "suspended";
        break;
    case 'I':
        m_state = "invalid";
        m_valid = false;
        break;
    case 'S':
        m_state = "suspended";
        break;
    case 'd':
        m_state = "no table";
        break;
    case 'i':
        m_state = "inactive table";
        break;
    default:
        m_state = "unknown";
    }

    if (!additional_state.isEmpty())
        m_state = additional_state + " / " + m_state;

    if (m_lv_name.contains("_mlog", Qt::CaseSensitive)) {
        m_lvmmirror_log = true;    // this needs to be here in case it is a mirrored mirror log
        m_lv_fs = "";
    } else if (m_lv_name.contains("_mimagetmp_", Qt::CaseSensitive)) {
        m_temp = true; 
        m_lv_fs = "";
    } else if (!m_lvmmirror_log && !isMirrorLeg() && !m_virtual && !m_metadata && !m_thin_pool) {
        m_lv_fs = fsprobe_getfstype2(m_lv_mapper_path);
        m_lv_fs_label = fsprobe_getfslabel(m_lv_mapper_path);
        m_lv_fs_uuid  = fsprobe_getfsuuid(m_lv_mapper_path);
    } else {
        m_lv_fs = "";
        m_lv_fs_label = "";
        m_lv_fs_uuid  = "";
    }

    if (m_cow_snap || m_merging) {
        value = lvm_lv_get_property(lvmLv, "origin");
        if (value.is_valid)
            m_origin = value.value.string;

        value = lvm_lv_get_property(lvmLv, "snap_percent");
        if (value.is_valid)
            m_snap_percent = lvm_percent_to_float(value.value.integer);
        else
            m_snap_percent = 0;
    } else if (m_thin || m_thin_pool) {    // Calling up snap_percent on thin snaps causes segfaults
        value = lvm_lv_get_property(lvmLv, "data_percent");
        if (value.is_valid)
            m_data_percent = lvm_percent_to_float(value.value.integer);
    } else {
        m_origin = "";
    }

    if ((m_lvmmirror_leg || m_lvmmirror_log)) {
        if (m_lvmmirror_log && m_lvmmirror)
            m_type = QString("log mirror");

        if (m_lvmmirror_log && m_lvmmirror_leg)
            m_type = QString("log image");
        else if ((m_lvmmirror || m_virtual) && !m_lvmmirror_log)
            m_lvmmirror_leg = true;
    }

    value = lvm_lv_get_property(lvmLv, "copy_percent");
    if (value.is_valid)
        m_copy_percent = lvm_percent_to_float(value.value.integer);
    else
        m_copy_percent = 0;

    if (m_snap_container && !was_snap_container)
        m_uuid = QUuid::createUuid().toString();
    else if (!m_snap_container) 
        m_uuid = lvm_lv_get_property(lvmLv, "lv_uuid").value.string;
    
    processMounts();
    
    if (m_snap_container) {
        m_type = "origin";
        
        if(flags.size() > 6){
            if (flags[6] == 'r')
                m_raid = true;
            else if (flags[6] == 't')
                m_thin = true;
        }
    } else if (m_type.contains("origin", Qt::CaseInsensitive) && !m_snap_container) {
        if(flags.size() > 6){
            if (flags[6] == 'r')
                m_raid = true;
            else if (flags[6] == 't')
                m_thin = true;
        }
        
        if (m_thin)
            m_type = "thin volume";
        else
            m_type = m_segments[0]->type;
    }
    
    insertChildren(lvmLv, lvmVg);
    countLegsAndLogs();
    calculateTotalSize();
}

void LogVol::setPolicy(const char flag2)
{
    m_alloc_locked = false;

    switch (flag2) {
    case 'C':
        m_alloc_locked = true;
    case 'c':
        m_policy = CONTIGUOUS;
        break;
    case 'L':
        m_alloc_locked = true;
    case 'l':
        m_policy = CLING;
        break;
    case 'N':
        m_alloc_locked = true;
    case 'n':
        m_policy = NORMAL;
        break;
    case 'A':
        m_alloc_locked = true;
    case 'a':
        m_policy = ANYWHERE;
        break;
    case 'I':
        m_alloc_locked = true;
    case 'i':
        if (m_vg->getPolicy() == CLING)
            m_policy = INHERIT_CLING;
        else if (m_vg->getPolicy() == ANYWHERE)
            m_policy = INHERIT_ANYWHERE;
        else if (m_vg->getPolicy() == CONTIGUOUS)
            m_policy = INHERIT_CONTIGUOUS;
        else
            m_policy = INHERIT_NORMAL;
        break;
    default:
        m_policy = NORMAL;
        break;
    }
}

void LogVol::setSnapContainer(vg_t lvmVg, lv_t lvmLv)
{
    QList<lv_t> lvm_child_snapshots(getLvmSnapshots(lvmVg));
    
    if ((!lvm_child_snapshots.isEmpty()) && m_lv_parent == nullptr) {
        m_snap_container = true;
        m_seg_total = 1;
    } else if ((!lvm_child_snapshots.isEmpty()) && m_lv_parent != nullptr) {
        if (m_lv_parent->isThinPool()) {
            m_snap_container = true;
            m_seg_total = 1;
        } else if (m_lv_parent->getFullName() != getFullName() ) {
            m_snap_container = true;
            m_seg_total = 1;
        } else {
            m_snap_container = false;
            m_seg_total = lvm_lv_get_property(lvmLv, "seg_count").value.integer;
        } 
    } else {
        m_snap_container = false;
        m_seg_total = lvm_lv_get_property(lvmLv, "seg_count").value.integer;
    }
}    

void LogVol::processMounts()
{
    m_mount_entries = m_tables->getMtabEntries(m_major_device, m_minor_device);

    QStringList mpts;

    for (auto entry : m_mount_entries)
        mpts << entry->getMountPoint();

    m_mounted = !mpts.isEmpty();

    if (m_mounted) {
        const FSData fs_data = get_fs_data(mpts[0]);
        if (fs_data.size > 0) {
            m_fs_size = fs_data.size * fs_data.block_size;
            m_fs_used = fs_data.used * fs_data.block_size;
        } else {
            m_fs_size = -1;
            m_fs_used = -1;
        }
    } else {
        m_fs_size = -1;
        m_fs_used = -1;
    }
}

void LogVol::insertChildren(lv_t lvmLv, vg_t lvmVg)
{
    lv_t lvm_child;
    QList<lv_t> lvm_child_snapshots(getLvmSnapshots(lvmVg));

    m_lv_children.clear();

    if (m_snap_container) {
        for (const auto snap : lvm_child_snapshots)
            m_lv_children << SmrtLvPtr(new LogVol(snap, lvmVg, m_vg, this, m_tables));

        m_lv_children << SmrtLvPtr(new LogVol(lvmLv, lvmVg, m_vg, this, m_tables));
    } else {
        QStringList names;
        names << removePvNames() << getMetadataNames() << getPoolVolumeNames(lvmVg);

        if (m_lvmmirror && !m_log.isEmpty())
            names << m_log;

        names.removeDuplicates();

        for (const auto name : names) {
            QByteArray qba = name.toLocal8Bit();
            lvm_child = lvm_lv_from_name(lvmVg, qba.data());
            
            if (lvm_child) {
                QByteArray flags(lvm_lv_get_property(lvm_child, "lv_attr").value.string);
                if (!flags.isEmpty() && flags[0] !='-') // filters out normal volumes which are never children
                    m_lv_children << SmrtLvPtr(new LogVol(lvm_child, lvmVg, m_vg, this, m_tables));
            }
        }
    }
}

void LogVol::countLegsAndLogs()
{
    m_mirror_count = 0;
    m_log_count = 0;

    if (m_lvmmirror) {
        for (const auto lv : getAllChildrenFlat()) { 
            if (lv->isLvmMirrorLeg() && !lv->isMirror() && !lv->isLvmMirrorLog())
                m_mirror_count++;

            if (lv->isLvmMirrorLog() && !lv->isMirror())
                m_log_count++;
        }
    } else if (getRaidType() == 1) {
        for (const auto lv : getAllChildrenFlat()) { 
            if (lv->isRaidImage())
                m_mirror_count++;
        }
    } else {
        m_mirror_count = 1;  // linear volumes count as mirror = 1;
    }
}

QList<lv_t> LogVol::getLvmSnapshots(vg_t lvmVg)
{
    lvm_property_value value;
    dm_list     *lv_dm_list = lvm_vg_list_lvs(lvmVg);
    lvm_lv_list *lv_list;
    QList<lv_t>  lvm_snapshots;

    if (lv_dm_list) {
        dm_list_iterate_items(lv_list, lv_dm_list) {
            value = lvm_lv_get_property(lv_list->lv, "origin");
            if (QString(value.value.string).trimmed() == m_lv_name)
                lvm_snapshots << lv_list->lv;
        }
    }

    return lvm_snapshots;
}

QStringList LogVol::getPoolVolumeNames(vg_t lvmVg) // Excluding snapshots -- they go under the origins
{
    lvm_property_value value;
    dm_list *const lv_dm_list = lvm_vg_list_lvs(lvmVg);
    lvm_lv_list *lv_list;
    QStringList names;

    if (lv_dm_list) {
        dm_list_iterate_items(lv_list, lv_dm_list) {
            value = lvm_lv_get_property(lv_list->lv, "origin");
            if (value.is_valid) {
                if (QString(value.value.string) == "") {
                    value = lvm_lv_get_property(lv_list->lv, "pool_lv");
                    if (value.is_valid) {
                        if (QString(value.value.string).trimmed() == m_lv_name) {
                            value = lvm_lv_get_property(lv_list->lv, "lv_name");
                            if (value.is_valid)
                                names << QString(value.value.string).trimmed();
                        }
                    }
                }
            }
        }
    }

    return names;
}

// Finds logical volumes that are children of this volume by
// removing physical volumes from the list along with pvmove
// volumes. One pvmove can be under several lvs so isn't
// really a child.

QStringList LogVol::removePvNames()
{
    QStringList names(getPvNamesAll());

    for (int i = names.size() - 1; i >= 0; i--) {
        if (names[i].startsWith("pvmove"))
            names.removeAt(i);
    }

    for (const auto pv : m_vg->getPhysicalVolumes()) {
        for (int i = names.size() - 1; i >= 0; i--) {
            if (pv->getMapperName() == names[i])
                names.removeAt(i);
        }
    }

    return names;
}

// Finds metadata child sub volumes of this volume
// TODO -- ophans can be named this way too.
// Rewrite to filter them rather than doing it in
// insertChildren()
QStringList LogVol::getMetadataNames()
{
    QStringList children;

    for (const auto lv : m_vg->getLvNamesAll()) {
        if (lv.contains("_rmeta_")) {
            if (m_lv_name == lv.left(lv.indexOf("_rmeta_")))
                children << lv;
        } else if (lv.endsWith("_tmeta") && m_thin_pool) {
            if (m_lv_name == lv.left(lv.indexOf("_tmeta")))
                children << lv;
        }
    }

    return children;
}

void LogVol::calculateTotalSize()
{
    m_total_size = 0;

    if (!m_thin_pool && m_lv_children.size()) {
        for (const auto &child : m_lv_children)
            m_total_size += child->getTotalSize();
    } else if (m_thin_pool) {
        for (const auto &child : m_lv_children) {
            if (child->isThinPoolData() || child->isMetadata())
                m_total_size += child->getTotalSize();
        }
    } else {
        m_total_size = m_size;
    }
}

void LogVol::processSegments(lv_t lvmLv, QByteArray flags)
{
    lvm_property_value value;
    dm_list* lvseg_dm_list = lvm_lv_list_lvsegs(lvmLv);
    lvm_lvseg_list *lvseg_list;
    lvseg_t lvm_lvseg;

    while (m_segments.size())
        delete m_segments.takeAt(0);

    if (lvseg_dm_list) {
        dm_list_iterate_items(lvseg_list, lvseg_dm_list) {
            lvm_lvseg = lvseg_list->lvseg;

            Segment *segment = new Segment();
            value = lvm_lvseg_get_property(lvm_lvseg, "segtype");
            if (value.is_valid)
                segment->type = value.value.string;

            if (segment->type == QString("mirror")) {
                m_lvmmirror = true;
                segment->type = QString("lvm mirror");
            } else if (segment->type == QString("raid1")) {
                m_raidmirror = true;
                segment->type = QString("raid1 mirror");
            }

            const bool pvmove = (flags[0] == 'p');  // use m_pvmove if processSegments is moved to after the main section

            if ((m_lvmmirror || m_raidmirror) && !pvmove) {
                segment->stripes = 1;
                segment->stripe_size = 1;
                segment->size = 1;
            } else if (pvmove) { 
                segment->stripes = 1;
                segment->stripe_size = 1;
                value = lvm_lvseg_get_property(lvm_lvseg, "seg_size");
                if (value.is_valid)
                    segment->size = value.value.integer;
            } else {
                value = lvm_lvseg_get_property(lvm_lvseg, "stripes");
                if (value.is_valid)
                    segment->stripes = value.value.integer;
                
                if (MasterList::isLvmVersionEqualOrGreater("2.02.100")) { // stripe_size bug fixed
                    if (MasterList::isLvmVersionEqualOrGreater("2.02.164")) { 
                        value = lvm_lvseg_get_property(lvm_lvseg, "stripe_size");
                        if (value.is_valid)
                            segment->stripe_size = value.value.integer;
                    } else {
                        value = lvm_lvseg_get_property(lvm_lvseg, "stripesize");
                        if (value.is_valid)
                            segment->stripe_size = value.value.integer;
                    }
                } else {
                    value = lvm_lvseg_get_property(lvm_lvseg, "stripesize");
                    if (value.is_valid)
                        segment->stripe_size = value.value.integer * 512;
                }

                value = lvm_lvseg_get_property(lvm_lvseg, "seg_size");
                if (value.is_valid)
                    segment->size = value.value.integer;
            }

            QString raw_paths;
            QStringList devices_and_starts;
            value = lvm_lvseg_get_property(lvm_lvseg, "devices");
            if (value.is_valid)
                raw_paths = value.value.string;

            if (raw_paths.size()) {
                for (auto dev : raw_paths.split(',')) {
                    QStringList temp = dev.split('(');
                    segment->device_path.append(findMapperPath(temp[0]));
                    segment->starting_extent.append((temp[1].remove(')')).toLongLong());
                }
            }

            if (flags[0] == 't') {
                
                value = lvm_lvseg_get_property(lvm_lvseg, "chunksize");
                if (value.is_valid) {

                    if (MasterList::isLvmVersionEqualOrGreater("2.02.100"))  // chunksize bug fixed in v2.2.100 
                        segment->chunk_size = value.value.integer;
                    else 
                        segment->chunk_size = value.value.integer * 512;
                    
                } else {
                    segment->chunk_size = 0;
                }
            } else {
                segment->chunk_size = 0;
            }

            if (MasterList::isLvmVersionEqualOrGreater("2.02.109")) {  // discards bug fixed by v2.2.109 
                if (flags[0] == 't') {
                    value = lvm_lvseg_get_property(lvm_lvseg, "discards");
                    if (value.is_valid)
                        segment->discards = value.value.string;
                }
            }
            
            m_segments.append(segment);
        }
    }
}

LvList LogVol::getAllChildrenFlat() const
{
    LvList flat_list = getChildren();

    for (const auto child : m_lv_children)
        flat_list << child->getAllChildrenFlat();

    return flat_list;
}

LvList LogVol::getChildren() const 
{ 
    LvList children;

    for (auto child : m_lv_children) {
        children << child.data();
    }

    return children; 
}
 
LvList LogVol::getSnapshots() const
{
    LvList snapshots;
    const LogVol *cntr = this;

    if (cntr->getParent() != nullptr && !cntr->isSnapContainer()) {
        if (cntr->getFullName() == cntr->getParent()->getFullName())
            cntr = cntr->getParent();
    }

    if (cntr->isSnapContainer()) {
        snapshots = cntr->getChildren();

        for (int i = snapshots.size() - 1; i >= 0; i--) { // delete the 'real' lv leaving the snaps
            if (m_lv_name == snapshots[i]->getName())
                snapshots.removeAt(i);
        }
    }

    return snapshots;
}

LvList LogVol::getThinVolumes() const  // not including snap containers
{
    LvList vols;

    if (m_thin_pool) {
        for (const auto lv : getAllChildrenFlat()) {
            if (lv->isThinVolume() && !lv->isSnapContainer())
                vols << lv;
        }
    }

    return vols;
}

LvList LogVol::getThinDataVolumes() const
{
    LvList data;

    if (m_thin_pool) {
        for (const auto lv : getChildren()) {
            if (lv->isThinPoolData())
                data << lv;
        }
    }

    return data;
}

LvList LogVol::getThinMetadataVolumes() const
{
    LvList meta;

    if (m_thin_pool) {
        for (const auto lv : getChildren()) {
            if (lv->isThinMetadata())
                meta << lv;
        }
    }

    return meta;
}

LvList LogVol::getRaidImageVolumes() const
{
    LvList images;

    if (m_raid) {
        for (const auto lv : getAllChildrenFlat()) {
            if (lv->isRaidImage())
                images << lv;
        }
    }

    return images;
}

LvList LogVol::getRaidMetadataVolumes() const
{
    LvList meta;

    if (m_raid) {
        for (const auto lv : getAllChildrenFlat()) {
            if (lv->isRaidMetadata())
                meta << lv;
        }
    }

    return meta;
}

// Returns the mirror than owns this mirror leg or mirror log. Returns
// nullptr if this is not part of a mirror volume.
LogVol * LogVol::getParentMirror()
{
    LogVol *mirror = this;

    if (isLvmMirrorLog() || isLvmMirrorLeg() || isTemporary()) {

        if (isLvmMirrorLog() && isLvmMirrorLeg() && mirror->getParent()) // mirrored mirror log
            mirror = getParent()->getParent();
        else if (isLvmMirrorLog() || isLvmMirrorLeg())
            mirror = getParent();
        
        if (mirror && mirror->isTemporary())  // under conversion temp mirror
            mirror = mirror->getParent();

    } else if (isMirrorLeg()) {  // it's raid
        mirror = getParent();
    } else {
        mirror = nullptr;
    }

    return mirror;
}

// Returns the RAID volume than owns this RAID component
// nullptr if this is not part of a RAID volume.
LogVol * LogVol::getParentRaid()
{
    if (m_raid_metadata || m_raid_image)
        return getParent();
    else
        return nullptr;
}

QStringList LogVol::getPvNamesAll() const
{
    QStringList pv_names;

    for (const auto seg : m_segments)
        pv_names << seg->device_path;

    pv_names.sort();
    pv_names.removeDuplicates();

    return pv_names;
}

QStringList LogVol::getPvNamesAllFlat() const
{
    QStringList pv_names;

    if (isRaidMetadata()) {
        pv_names << getPvNamesAll();
    } else if (m_snap_container || m_lvmmirror || m_raid) {
        for (auto child : getChildren())
            pv_names << child->getPvNamesAllFlat();
    } else if (m_thin_pool) {
        for (auto child : getChildren()) {
            if (child->isThinPoolData() || child->isThinMetadata())
                pv_names << child->getPvNamesAllFlat();
        }
    } else {
        pv_names << getPvNamesAll();
    }

    pv_names.sort();
    pv_names.removeDuplicates();
    
    return pv_names;
}

long long LogVol::getSpaceUsedOnPv(const QString pvname) const
{
    long long space_used = 0;

    if (m_thin_pool) {
        for (auto data : getThinDataVolumes())
            space_used += data->getSpaceUsedOnPv(pvname);

        for (auto meta : getThinMetadataVolumes())
            space_used += meta->getSpaceUsedOnPv(pvname);
    } else if (m_raid && !(m_raid_metadata || m_raid_image)) {
        for (auto image : getRaidImageVolumes())
            space_used += image->getSpaceUsedOnPv(pvname);

        for (auto meta : getRaidMetadataVolumes())
            space_used += meta->getSpaceUsedOnPv(pvname);
    } else {
        for (auto seg : m_segments) {
            for (auto path : seg->device_path) {
                if (path == pvname)
                    space_used += (seg->size) / (seg->stripes) ;
            }
        }
    }

    return space_used;
}

long long LogVol::getMissingSpace() const
{
    LvList const children = getChildren();
    long long missing = 0;

    if (isPartial()) {
        if (children.isEmpty()) {
            for (const auto pvname : getPvNamesAllFlat()) {
                PhysVol *const pv = m_vg->getPvByName(pvname);
                if (pv && !pv->isMissing())
                    missing -= getSpaceUsedOnPv(pvname);
            }
        } else {
            missing = 0;
            for (auto child : children)
                missing += child->getMissingSpace();
        }
    } else {
        missing = 0;
    }

    return missing;
}

long long LogVol::getChunkSize(int segment) const
{
    if (segment > m_segments.size() - 1)
        segment = 0;

    return m_segments[segment]->chunk_size;
}

int LogVol::getRaidType() const
{
    int type;
    QRegExp reg("[0-9]+");
    QStringList matches;

    // This needs to be fixed so changes in the string
    // m_type can't screw with it.

    if (m_raid && reg.indexIn(m_type) >= 0) {
        matches = reg.capturedTexts();
        type = matches[0].toInt();
    } else {
        type = -1;
    }

    return type;
}

QString LogVol::getDiscards(int segment) const
{
    if (segment > m_segments.size() - 1)
        segment = 0;

    return m_segments[segment]->discards;
}

double LogVol::getSnapPercent() const
{
    if (m_cow_snap || m_merging) {
        if (m_active)
            return m_snap_percent;
        else
            return -1;
    } else {
        return 0.0;
    }
}

double LogVol::getCopyPercent() const
{
    if (m_active)
        return m_copy_percent;
    else
        return -1;
}

double LogVol::getDataPercent() const
{
    if (m_active)
        return m_data_percent;
    else
        return -1;
}

bool LogVol::isSynced() const
{
    if (m_synced) {
        for (auto child : getChildren()) {
            if (!child->isSynced())
                return false;
        }

        return true;        
    } else {
        return false;
    }
}

int LogVol::getSegmentStripes(const int segment)
{
    return m_segments[segment]->stripes;
}

int LogVol::getSegmentStripeSize(const int segment)
{
    return m_segments[segment]->stripe_size;
}

long long LogVol::getSegmentSize(const int segment)
{
    return m_segments[segment]->size;
}

long long LogVol::getSegmentExtents(const int segment)
{
    return (m_segments[segment]->size / m_vg->getExtentSize());
}

QList<long long> LogVol::getSegmentStartingExtent(const int segment)
{
    return m_segments[segment]->starting_extent;
}

QStringList LogVol::getPvNames(const int segment) const
{
    return m_segments[segment]->device_path;
}

// Returns the raid metadata volume associated with a raid image volume
LogVol * LogVol::getRaidImageMetadata() const
{
    if (isRaidImage()) {
        return m_vg->getLvByName(getName().replace(QString("_rimage_"), QString("_rmeta_")));
    } else {
        return nullptr;
    }
}

// Returns the raid image volume associated with a raid metadata volume
LogVol * LogVol::getRaidMetadataImage() const
{
    if (isRaidMetadata()) {
        return m_vg->getLvByName(getName().replace(QString("_rmeta_"), QString("_rimage_")));
    } else {
        return nullptr;
    }
}

QStringList LogVol::getMountPoints() const 
{ 
    QStringList mpts;

    for (auto entry : m_mount_entries)
        mpts << entry->getMountPoint();

    return mpts; 
}

void LogVol::getDeviceNumbers(unsigned long &majornum, unsigned long &minornum)
{ 
    majornum = 0;
    minornum = 0;

    QFileInfo fi(m_lv_mapper_path);
    if (!fi.exists())
        return;
    
    QByteArray qba = fi.canonicalFilePath().toLocal8Bit();
    struct stat fs;
    if (stat(qba.data(), &fs))  // error
        return;

    majornum = major(fs.st_rdev); 
    minornum = minor(fs.st_rdev);
}
