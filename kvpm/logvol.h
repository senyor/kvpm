/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef LOGVOL_H
#define LOGVOL_H

#include <lvm2app.h>

#include <QObject>
#include <QPointer>
#include <QStringList>

#include "allocationpolicy.h"
#include "misc.h"
#include "mountentry.h"
#include "mounttables.h"

class QWidget;

class LogVol;
class VolGroup;
class Segment;


// This class takes the handles for volumes provided
// by liblvm2app and converts the information into
// a more Qt/KDE friendly form.

typedef QPointer<LogVol> LogVolPointer;
typedef QList<LogVolPointer> LogVolList;

class LogVol : public QObject
{
    Q_OBJECT
    VolGroup *m_vg;
    QList<Segment *> m_segments;
    MountList m_mount_entries;

    LogVolList m_lv_children;  // For a mirror the children are the legs and log
                               // Snapshots are also children -- see m_snap_container
                               // RAID Metadata is here too
    MountTables *m_tables;
    LogVol *m_lv_parent;       // NULL if this is the 'top' lv
    QString m_lv_full_name;    // volume_group/logical_volume
    QString m_lv_name;         // name of this logical volume
    QString m_lv_mapper_path;  // full path to volume, ie: /dev/vg1/lvol1
    QString m_lv_fs;           // Filesystem on volume, if known
    QString m_lv_fs_label;     // Filesystem label or name
    QString m_lv_fs_uuid;      // Filesystem uuid
    QString m_origin;          // the origin if this is a snapshot
    QString m_pool;            // the name of the thin pool if this is a thin volume, otherwise empty

    QString m_log;           // The mirror log, if this is a mirror
    QString m_type;          // the type of volume
    QString m_state;         // the lv state
    AllocationPolicy m_policy;

    QString     m_uuid;
    QStringList m_tags;
    QString     m_fstab_mount_point;
    QStringList m_mount_points;  // empty if not mounted

    double  m_snap_percent;      // the percentage used, if this is a snapshot
    double  m_copy_percent;      // the percentage of extents moved, if pvmove underway
    double  m_data_percent;      // the percentage of extents used by thin volumes
    long long m_size;            // size in bytes
    long long m_total_size;      // size in bytes, size of all children (mirror legs/logs and snaps) added together
    long long m_extents;         // size in extents
    long long m_fs_size;         // fs size in bytes
    long long m_fs_used;         // bytes used up in fs

    int m_seg_total;             // total number of segments in logical volume
    unsigned long m_major_device; // Unix device major number, if set
    unsigned long m_minor_device; // Unix device minor number, if set
    int m_log_count;             // if a mirror -- how many logs
    int m_mirror_count;          // if mirror -- how many legs
    bool m_partial;              // Is missing one or more physical volumes
    bool m_zero;                 // Newly-allocated data blocks are overwritten with blocks of zeroes before use
    bool m_virtual;              // Is virtual volume
    bool m_under_conversion;     // Is going to be a mirrored volume
    bool m_raidmirror;           // Is a raid 1 mirrored volume
    bool m_raidmirror_leg;       // Is one of the underlying legs of a mirrored volume
    bool m_lvmmirror;            // Is an LVM (not raid 1) mirrored volume
    bool m_lvmmirror_leg;        // Is one of the underlying legs of a mirrored volume
    bool m_lvmmirror_log;        // Is the log for a mirrored volume
    bool m_metadata;             // Is RAID or thin pool metadata -- we get this from the lvs attribute flags
    bool m_raid_metadata;        // Is RAID metadata -- we get this from the name ie: *_rmeta_*
    bool m_thin_metadata;        // Is thin pool metadata -- we get this from the name ie: *_tmeta_*
    bool m_raid;                 // Is a raid volume, including raid 1 mirrors
    bool m_raid_image;           // Is a raid device under a raid volume
    bool m_fixed, m_persistent;  // fix the device minor and major number
    bool m_synced;               // mirror or raid fully synced
    bool m_alloc_locked;         // allocation type is fixed when pvmove is underway
                                 // (and maybe other times)
    bool m_active;
    bool m_mounted;              // has a mounted filesystem
    bool m_open;                 // device is open
    bool m_orphan;               // virtual device with no pvs
    bool m_pvmove;               // is a pvmove temporary volume
    bool m_cow_snap;             // is a traditional snapshot volume
    bool m_thin_snap;            // is a thin snapshot volume
    bool m_snap_container;       // is a fake lv that contains the real lv and its snapshots as children
    bool m_is_origin;            // is the origin of a 'COW' type snap. 
    bool m_writable;
    bool m_valid;                // is a valid snap
    bool m_merging;              // is snap or snap origin that is merging
    bool m_thin;                 // is thin volume or a thin snapshot
    bool m_thin_data;            // is thin pool data
    bool m_thin_pool;
    bool m_temp;                 // A temporary mirror created while converting a mirror

    void countLegsAndLogs();
    void processSegments(lv_t lvmLV, const QByteArray flags);
    QStringList removePvNames();     // list 'devices' that are really sub lvs
    QStringList getMetadataNames();  // names of sub lvs that are metadata for this lv
    QStringList getPoolVolumeNames(vg_t lvmVG);
    QList<lv_t> getLvmSnapshots(vg_t lvmVG);
    void insertChildren(lv_t lvmLV, vg_t lvmVG);
    void calculateTotalSize();

public:
    LogVol(lv_t lvmLV, vg_t lvmVG, VolGroup *const vg, LogVol *const lvParent, 
           MountTables *const tables, bool orphan = false);
    ~LogVol();

    void rescan(lv_t lvmLV, vg_t lvmVG);
    LogVolList getChildren() const { return m_lv_children; } // just the children -- not grandchildren etc.
    LogVolList getAllChildrenFlat();  // All children, grandchildren etc. un-nested.
    LogVolList getSnapshots();        // This will work the same for snapcontainers or the real lv
    LogVolList getThinVolumes();      // Thin logical volumes under a thin pool
    LogVolList getThinDataVolumes();      // Data volumes supporting a thin pool
    LogVolList getThinMetadataVolumes();  // Metadata volumes for a thin pool
    LogVolList getRaidImageVolumes();     // Image volumes supporting a RAID volume
    LogVolList getRaidMetadataVolumes();  // Metadata for a RAID volume
    LogVolList takeChildren();        // removes the children from the logical volume
    LogVolPointer getParent() const { return m_lv_parent; }   // NULL if this is a "top level" lv
    LogVolPointer getParentMirror();             // NULL if this is not an lvm type mirror component
    LogVolPointer getParentRaid();               // NULL if this is not a RAID type component
    LogVolPointer getRaidImageMetadata();        // NULL if this is not a RAID image
    LogVolPointer getRaidMetadataImage();        // NULL if this is not RAID metadata
    VolGroup* getVg() const { return m_vg; }
    QString getName() const { return m_lv_name; }
    QString getPoolName() const { return m_pool; } // Name of this volume's thin pool if it is a thin volume, empty otherwise
    QString getFullName() const { return m_lv_full_name; }
    QString getFilesystem() const { return m_lv_fs; }
    QString getFilesystemLabel() const { return m_lv_fs_label; }
    QString getFilesystemUuid() const { return m_lv_fs_uuid; }
    QString getMapperPath() const { return m_lv_mapper_path; }
    AllocationPolicy getPolicy() const { return m_policy; }
    QString getState() const { return m_state; }
    QString getType() const { return m_type; }
    int getRaidType();
    QString getOrigin() const { return  m_origin; }        // The name of the parent volume to a snapshot
    QString getUuid() const { return m_uuid; }
    int getSegmentCount() const { return m_seg_total; }
    int getSegmentStripes(const int segment);
    int getSegmentStripeSize(const int segment);
    long long getSegmentSize(const int segment);
    long long getSegmentExtents(const int segment);
    QList<long long> getSegmentStartingExtent(const int segment);
    QStringList getPvNames(const int segment);
    QStringList getPvNamesAll();         // full path of physical volumes for all segments
    QStringList getPvNamesAllFlat();     // full path of physical volumes including child lvs, un-nested
    QStringList getMountPoints() const { return m_mount_points; }
    MountList getMountEntries() const { return m_mount_entries; }
    QString getFstabMountPoint() const { return m_fstab_mount_point; }
    QStringList getTags() const { return m_tags; }
    QString getDiscards(int segment);
    long long getSpaceUsedOnPv(const QString pvname);
    long long getMissingSpace();  // space used on pvs that are missing
    long long getChunkSize(int segment);
    long long getExtents() const { return m_extents; }
    long long getSize() const { return m_size; }
    long long getTotalSize() const { return m_total_size; }
    long long getFilesystemSize() const { return m_fs_size; }
    long long getFilesystemUsed() const { return m_fs_used; }
    double getSnapPercent();
    double getCopyPercent();
    double getDataPercent();
    unsigned long getMinorDevice() const { return m_minor_device; }
    unsigned long getMajorDevice() const { return m_major_device; }
    int getLogCount() const { return m_log_count; }      // RAID 1 returns 0 since it doesn't have separate logs
    int getMirrorCount() const { return m_mirror_count; }
    int getSnapshotCount() { return getSnapshots().size(); }
    bool isActive() const { return m_active; }
    bool isFixed() const { return m_fixed; }
    bool isLocked() const { return m_alloc_locked; }
    bool isMerging() const { return m_merging; }
    bool isMetadata() const { return m_metadata; }
    bool isRaidMetadata() const { return m_raid_metadata; }
    bool isThinMetadata() const { return m_thin_metadata; }
    bool isMirror() const { return (m_lvmmirror || m_raidmirror); }
    bool isMirrorLeg() const { return (m_lvmmirror_leg || m_raidmirror_leg); }
    bool isLvmMirror() const { return m_lvmmirror; }
    bool isLvmMirrorLeg() const { return m_lvmmirror_leg; }
    bool isLvmMirrorLog() const { return m_lvmmirror_log; }
    bool isMounted() const { return m_mounted; }
    bool isOpen() const { return m_open; }
    bool isCowOrigin() const { return m_is_origin; }
    bool isOrphan() const { return m_orphan; }
    bool isPersistent() const { return m_persistent; }
    bool isPvmove() const { return m_pvmove; }
    bool isRaid() const { return m_raid; }
    bool isRaidImage() const { return m_raid_image; }
    bool isCowSnap() const { return m_cow_snap; }
    bool isThinSnap() { return m_thin_snap; }
    bool isSnapContainer() { return m_snap_container; }
    bool isSynced();
    bool isTemporary() const { return m_temp; }
    bool isThinVolume() const { return m_thin; }
    bool isThinPool() const { return m_thin_pool; }
    bool isThinPoolData() const { return m_thin_data; }
    bool isUnderConversion() const { return m_under_conversion; }
    bool isValid() const { return m_valid; }
    bool isVirtual() const { return m_virtual; }
    bool isWritable() const { return m_writable; }
    bool isPartial() const { return m_partial; }
    bool willZero() const { return m_zero; }
};

#endif
