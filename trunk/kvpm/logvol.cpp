/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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

#include <QDebug>
#include <QRegExp>
#include <QUuid>
#include <QWidget>

#include "fsdata.h"
#include "fsprobe.h"
#include "mountentry.h"
#include "mounttables.h"
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
};

LogVol::LogVol(lv_t lvmLV, vg_t lvmVG, VolGroup *const vg, LogVol *const lvParent, MountTables *const tables, bool orphan) :
    m_vg(vg),
    m_lv_parent(lvParent),
    m_orphan(orphan)
{
    m_snap_container   = false;
    m_tables = tables;
    rescan(lvmLV, lvmVG);
}

LogVol::~LogVol()
{
    QList<LogVol *> children = getChildren();

    for (int x = m_mount_entries.size() - 1; x >= 0; x--)
        delete m_mount_entries.takeAt(x);

    while (children.size())
        delete children.takeAt(0);

    while (m_segments.size())
        delete m_segments.takeAt(0);
}

void LogVol::rescan(lv_t lvmLV, vg_t lvmVG)
{
    QString additional_state;
    QByteArray flags;
    lvm_property_value value;
    bool was_snap_container = m_snap_container;
    m_snap_container   = false;
    m_under_conversion = false;
    m_is_origin   = false;
    m_merging     = false;
    m_metadata    = false;
    m_raidmirror     = false;
    m_raidmirror_leg = false;
    m_lvmmirror      = false;
    m_lvmmirror_leg  = false;
    m_lvmmirror_log  = false;
    m_raid        = false;
    m_raid_image  = false;
    m_snap        = false;
    m_thin        = false;
    m_pvmove      = false;
    m_valid       = true;
    m_virtual     = false;

    value = lvm_lv_get_property(lvmLV, "lv_name");
    m_lv_name = QString(value.value.string).trimmed();
    m_lv_full_name = m_vg->getName() + '/' + m_lv_name;

    value = lvm_lv_get_property(lvmLV, "lv_attr");
    flags.append(value.value.string);

    processSegments(lvmLV, flags);

    value = lvm_lv_get_property(lvmLV, "lv_path");
    m_lv_mapper_path = QString(value.value.string).trimmed();

    value = lvm_lv_get_property(lvmLV, "mirror_log");
    m_log = QString(value.value.string).trimmed();

    QList<lv_t> lvm_child_snapshots;
    lvm_child_snapshots.append(getLvmSnapshots(lvmVG));

    if ((! lvm_child_snapshots.isEmpty()) && m_lv_parent == NULL) {
        m_snap_container = true;
        m_seg_total = 1;
    } else {
        m_snap_container = false;
        value = lvm_lv_get_property(lvmLV, "seg_count");
        m_seg_total = value.value.integer;
    }

    switch (flags[0]) {
    case 'c':
        m_type = "under conversion";
        m_lvmmirror = true;
        m_under_conversion = true;
        break;
    case 'e':
        m_type = "metadata";
        m_metadata = true;
        break;
    case 'I':
        if (flags[6] == 'r'){
            if (m_lv_parent != NULL) {
                if (m_lv_parent->isRaid() && m_lv_parent->getRaidType() == 1) {
                    m_type = "mirror leg";
                    m_raidmirror_leg = true;
                } else
                    m_type = "raid image";
            }
            else
                m_type = "raid image";

            m_raid_image = true;
        } else {
            m_type = "mirror leg";
            m_lvmmirror_leg = true;
        }

        additional_state = "un-synced";
        break;
    case 'i':
        if (flags[6] == 'r'){
            if (m_lv_parent != NULL) {
                if (m_lv_parent->isRaid() && m_lv_parent->getRaidType() == 1) {
                    m_type = "mirror leg";
                    m_raidmirror_leg = true;
                } else
                    m_type = "raid image";
            }
            else
                m_type = "raid image";

            m_raid_image = true;
        } else {
            m_type = "mirror leg";
            m_lvmmirror_leg = true;
        }
        
        additional_state = "synced";
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
        m_snap = true;
        break;
    case 'S':
        if (flags[4] != 'I') {         // When 'S' stops getting used for Invalid and only merging - remove this
            m_type = "snapshot";
            additional_state = "merging";
            m_snap = true;
            m_merging = true;
        } else {
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

    switch (flags[1]) {
    case 'w':
        m_writable = true;
        break;
    default:
        m_writable = false;
    }

    m_alloc_locked = false;

    switch (flags[2]) {
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

    if (m_alloc_locked)
        m_policy.append(" (locked)");

    switch (flags[3]) {
    case 'm':
        m_fixed = true;
        break;
    default:
        m_fixed = false;
    }

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

    switch (flags[5]) {
    case 'o':
        m_open = true;
        break;
    default:
        m_open = false;
    }

    if (m_lv_name.contains("_mlog", Qt::CaseSensitive)) {
        m_lvmmirror_log = true;    // this needs to be here in case it is a mirrored mirror log
        m_lv_fs = "";
    } else if (m_lv_name.contains("_mimagetmp_", Qt::CaseSensitive)) {
        m_virtual = true;    // This is to get lvactionsmenu to forbid doing anything to it
        m_lv_fs = "";
    } else if (!m_lvmmirror_log && !m_lvmmirror_leg && !m_virtual) {
        m_lv_fs = fsprobe_getfstype2(m_lv_mapper_path);
        m_lv_fs_label = fsprobe_getfslabel(m_lv_mapper_path);
        m_lv_fs_uuid  = fsprobe_getfsuuid(m_lv_mapper_path);
    } else {
        m_lv_fs = "";
        m_lv_fs_label = "";
        m_lv_fs_uuid  = "";
    }

    value = lvm_lv_get_property(lvmLV, "lv_size");
    m_size = value.value.integer;
    m_extents = m_size / m_vg->getExtentSize();

    if (m_snap || m_merging) {
        value = lvm_lv_get_property(lvmLV, "origin");
        m_origin = value.value.string;
        value = lvm_lv_get_property(lvmLV, "snap_percent");
        if (value.is_valid)
            m_snap_percent = (double)value.value.integer / 1.0e+6;
        else
            m_snap_percent = 0;
    } else
        m_origin = "";

    if ((m_lvmmirror_leg || m_lvmmirror_log)) {
        if (m_lvmmirror_log && m_lvmmirror)
            m_type = QString("log mirror");

        if (m_lvmmirror_log && m_lvmmirror_leg)
            m_type = QString("log image");
        else if ((m_lvmmirror || m_virtual) && !m_lvmmirror_log)
            m_lvmmirror_leg = true;
    }

    value = lvm_lv_get_property(lvmLV, "copy_percent");
    if (value.is_valid)
        m_copy_percent = (double)value.value.integer / 1.0e+6;
    else
        m_copy_percent = 0;

    value = lvm_lv_get_property(lvmLV, "lv_kernel_major");
    m_major_device = value.value.integer;
    value = lvm_lv_get_property(lvmLV, "lv_kernel_minor");
    m_minor_device = value.value.integer;

    value = lvm_lv_get_property(lvmLV, "lv_major");
    if((int64_t)value.value.integer != -1)
        m_persistent = true;
    else
        m_persistent = false;

    if (m_snap_container && !was_snap_container) {
        m_uuid = QUuid::createUuid().toString();
    } else if (!m_snap_container) {
        value = lvm_lv_get_property(lvmLV, "lv_uuid");
        m_uuid  = value.value.string;
    }

    value = lvm_lv_get_property(lvmLV, "lv_tags");
    QString tag = value.value.string;
    m_tags = tag.split(',', QString::SkipEmptyParts);

    for (int x = m_mount_entries.size() - 1; x >= 0; x--)
        delete m_mount_entries.takeAt(x);

    m_mount_entries = m_tables->getMtabEntries(m_major_device, m_minor_device);

    m_fstab_mount_point = m_tables->getFstabMountPoint(this);

    m_mount_points.clear();
    for (int x = 0; x < m_mount_entries.size(); x++)
        m_mount_points.append(m_mount_entries[x]->getMountPoint());

    m_mounted = !m_mount_points.isEmpty();

    if (m_mounted) {
        FSData fs_data = get_fs_data(m_mount_points[0]);
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

    if (m_snap_container) {
        m_type = "origin";
    } else if (m_type.contains("origin", Qt::CaseInsensitive) && !m_snap_container) {
        if(flags.size() > 6){
            if (flags[6] == 'm')
                m_raid = true;
            else if (flags[6] == 'r')
                m_raid = true;
            else if (flags[6] == 't')
                m_thin = true;
        }
        m_type = m_segments[0]->type;
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
    lvm_child_snapshots.append(getLvmSnapshots(lvmVG));

    while (m_lv_children.size())
        delete m_lv_children.takeAt(0);

    if (m_snap_container) {
        for (int x = lvm_child_snapshots.size() - 1; x >= 0; x--)
            m_lv_children.append(new LogVol(lvm_child_snapshots[x], lvmVG, m_vg, this, m_tables));

        m_lv_children.append(new LogVol(lvmLV, lvmVG, m_vg, this, m_tables));
    } else {
        child_name_list << removePvNames();
        child_name_list << getMetadataNames();

        if (m_lvmmirror && (!m_log.isEmpty()))
            child_name_list.append(m_log);

        for (int x = child_name_list.size() - 1; x >= 0; x--) {
            child_name = child_name_list[x].toLocal8Bit();
            lvm_child = lvm_lv_from_name(lvmVG, child_name.data());
            m_lv_children.append(new LogVol(lvm_child, lvmVG, m_vg, this, m_tables));
        }
    }
}

void LogVol::countLegsAndLogs()
{
    m_mirror_count = 0;
    m_log_count = 0;
    QList<LogVol *> all_lvs_flat = getAllChildrenFlat();
    LogVol *lv;

    if (m_lvmmirror) {
        for (int x = all_lvs_flat.size() - 1; x >= 0; x--) {
            lv = all_lvs_flat[x];

            if (lv->isLvmMirrorLeg() && !lv->isLvmMirror() && !lv->isLvmMirrorLog())
                m_mirror_count++;

            if (lv->isLvmMirrorLog() && !lv->isLvmMirror())
                m_log_count++;
        }
    } else if (getRaidType() == 1) {
        for (int x = all_lvs_flat.size() - 1; x >= 0; x--) {
            if (all_lvs_flat[x]->isRaidImage())
                m_mirror_count++;
        }
    } else
        m_mirror_count = 1;  // linear volumes count as mirror = 1;
}

QList<lv_t> LogVol::getLvmSnapshots(vg_t lvmVG)
{
    lvm_property_value value;
    dm_list     *lv_dm_list = lvm_vg_list_lvs(lvmVG);
    lvm_lv_list *lv_list;
    QList<lv_t>  lvm_snapshots;

    if (lv_dm_list) {
        dm_list_iterate_items(lv_list, lv_dm_list) {

            value = lvm_lv_get_property(lv_list->lv, "origin");
            if (QString(value.value.string).trimmed() == m_lv_name)
                lvm_snapshots.append(lv_list->lv);
        }
    }

    return lvm_snapshots;
}

// Finds logical volumes that are children of this volume by
// removing physical volumes from the list along with pvmove
// volumes. One pvmove can be under several lvs so isn't
// really a child.

QStringList LogVol::removePvNames()
{
    QStringList names(getPvNamesAll());

    for (int x = names.size() - 1; x >= 0; x--) {
        if (names[x].startsWith("pvmove"))
            names.removeAt(x);
    }

    QListIterator<PhysVol *> pv_itr(m_vg->getPhysicalVolumes());

    while (pv_itr.hasNext()) {
        PhysVol *const pv = pv_itr.next();

        for (int y = names.size() - 1; y >= 0; y--) {
            if (pv->getName() == names[y])
                names.removeAt(y);
        }

    }

    return names;
}

// Finds metadata child sub volumes of this volume

QStringList LogVol::getMetadataNames()
{
    QString lv;
    QStringList children;
    QStringList const names(m_vg->getLvNamesAll());
    QStringListIterator itr(names);

    while (itr.hasNext()){
        lv = itr.next();
        if (lv.contains("_rmeta_")){
            if (m_lv_name == lv.left(lv.indexOf("_rmeta_")))
                children << lv;
        }
    }

    return children;
}

void LogVol::calculateTotalSize()
{
    m_total_size = 0;

    if (m_lv_children.size()) {
        for (int x = m_lv_children.size() - 1; x >= 0; x--)
            m_total_size += m_lv_children[x]->getTotalSize();
    } else {
        m_total_size = m_size;
    }
}

void LogVol::processSegments(lv_t lvmLV, const QByteArray flags)
{
    // const QString version(lvm_library_get_version());
    Segment *segment;
    QStringList devices_and_starts, temp;
    QString raw_paths;
    lvm_property_value value;
    dm_list* lvseg_dm_list = lvm_lv_list_lvsegs(lvmLV);
    lvm_lvseg_list *lvseg_list;
    lvseg_t lvm_lvseg;

    while (m_segments.size())
        delete m_segments.takeAt(0);

    if (lvseg_dm_list) {
        dm_list_iterate_items(lvseg_list, lvseg_dm_list) {
            lvm_lvseg = lvseg_list->lvseg;

            if (flags[6] == 'm' && !(QString(flags[0]).contains(QRegExp("[rRiIl]"))))
                m_lvmmirror = true;
            else if (flags[6] == 'm' && (QString(flags[0]).contains(QRegExp("[rR]"))))
                m_raidmirror = true;
           
            segment = new Segment();

            if (m_lvmmirror)
                segment->type = QString("lvm mirror");
            else if (m_raidmirror)
                segment->type = QString("raid1 mirror");
            else {
                value = lvm_lvseg_get_property(lvm_lvseg, "segtype");
                if (value.is_valid)
                    segment->type = value.value.string;
            }

            if (m_lvmmirror || m_raidmirror) {
                segment->stripes = 1;
                segment->stripe_size = 1;
                segment->size = 1;
            } else {
                value = lvm_lvseg_get_property(lvm_lvseg, "stripes");
                if (value.is_valid)
                    segment->stripes = value.value.integer;

                /* Make multiplying by 512 depend on version number as defined
                   above when stripesize gets fixed in lvm2app. */

                value = lvm_lvseg_get_property(lvm_lvseg, "stripesize");
                if (value.is_valid)
                    segment->stripe_size = value.value.integer * 512;

                value = lvm_lvseg_get_property(lvm_lvseg, "seg_size");
                if (value.is_valid)
                    segment->size = value.value.integer;
            }

            value = lvm_lvseg_get_property(lvm_lvseg, "devices");
            if (value.is_valid) {
                raw_paths = value.value.string;
            }

            if (raw_paths.size()) {
                devices_and_starts = raw_paths.split(',');
                for (int x = 0; x < devices_and_starts.size(); x++) {
                    temp = devices_and_starts[x].split('(');
                    segment->device_path.append(temp[0]);
                    segment->starting_extent.append((temp[1].remove(')')).toLongLong());
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

    for (int x = 0; x < child_size; x++)
        flat_list.append(m_lv_children[x]->getAllChildrenFlat());

    return flat_list;
}

QList<LogVol *> LogVol::getSnapshots()
{
    QList<LogVol *> snapshots;
    LogVol *top_lv = this;

    while (top_lv->getParent() != NULL)
        top_lv = top_lv->getParent();

    if (top_lv->isSnapContainer()) {
        snapshots = top_lv->getChildren();

        for (int x = snapshots.size() - 1; x >= 0; x--) { // delete the 'real' lv leaving the snaps
            if (m_lv_name == snapshots[x]->getName())
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

QStringList LogVol::getPvNames(const int segment)
{
    return m_segments[segment]->device_path;
}

QStringList LogVol::getPvNamesAll()
{
    QStringList pv_names;
    QListIterator<Segment *> seg_itr(m_segments);

    while (seg_itr.hasNext())
        pv_names << seg_itr.next()->device_path;

    pv_names.sort();
    pv_names.removeDuplicates();

    return pv_names;
}

QStringList LogVol::getPvNamesAllFlat()
{
    if (m_snap_container || m_lvmmirror || m_raid) {

        QListIterator<LogVol *> child_itr(getChildren());
        QStringList pv_names;

        while (child_itr.hasNext()){
            pv_names << child_itr.next()->getPvNamesAllFlat();
        }

        pv_names.sort();
        pv_names.removeDuplicates();

        return pv_names;
    } else {
        return getPvNamesAll();
    }
}

VolGroup* LogVol::getVg()
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

long long LogVol::getSpaceUsedOnPv(const QString physicalVolume)
{
    long long space_used = 0;
    QListIterator<Segment *> seg_itr(m_segments);

    while (seg_itr.hasNext()) {

        Segment *const seg = seg_itr.next();
        QListIterator<QString> path_itr(seg->device_path);

        while (path_itr.hasNext()) {
            if (physicalVolume == path_itr.next())
                space_used += (seg->size) / (seg->stripes) ;
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

bool LogVol::isMetadata()
{
    return m_metadata;
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
    return (m_lvmmirror || m_raidmirror); 
}

bool LogVol::isMirrorLeg()
{
    return (m_lvmmirror_leg || m_raidmirror_leg); 
}

bool LogVol::isLvmMirror()
{
    return m_lvmmirror;
}

bool LogVol::isLvmMirrorLeg()
{
    return m_lvmmirror_leg;
}

bool LogVol::isLvmMirrorLog()
{
    return m_lvmmirror_log;
}

bool LogVol::isPersistent()
{
    return m_persistent;
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

bool LogVol::isRaid()
{
    return m_raid;
}

bool LogVol::isRaidImage()
{
    return m_raid_image;
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

bool LogVol::isThin()
{
    return m_thin;
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

int LogVol::getRaidType()
{
    int type;
    QRegExp reg("[0-9]+");
    QStringList matches;

    if (m_raid && reg.indexIn(m_type) >= 0) {
        matches = reg.capturedTexts();
        type = matches[0].toInt();
    }
    else
        type = -1;

    return type;
}

QStringList LogVol::getTags()
{
    return m_tags;
}

QString LogVol::getOrigin()
{
    return  m_origin;
}

QList<MountEntry *> LogVol::getMountEntries()
{
    QList<MountEntry *> copy;
    QListIterator<MountEntry *> itr(m_mount_entries);

    while (itr.hasNext())
        copy.append(new MountEntry(itr.next()));

    return copy;
}

QStringList LogVol::getMountPoints()
{
    return m_mount_points;
}

QString LogVol::getFstabMountPoint()
{
    return m_fstab_mount_point;
}

double LogVol::getSnapPercent()
{
    if (m_snap || m_merging) {
        if (m_active)
            return m_snap_percent;
        else
            return -1;
    }
    else
        return 0.0;
}

double LogVol::getCopyPercent()
{
    if (m_active)
        return m_copy_percent;
    else
        return -1;
}

QString LogVol::getUuid()
{
    return m_uuid;
}

bool LogVol::hasMissingVolume()
{
    return getPvNamesAllFlat().contains("unknown device");
}
