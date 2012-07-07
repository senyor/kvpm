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

#ifndef LOGVOL_H
#define LOGVOL_H

#include <lvm2app.h>

#include <QStringList>

class QWidget;

class MountEntry;
class MountTables;
class VolGroup;
class Segment;

// This class takes the handles for volumes provided
// by liblvm2app and converts the information into
// a more Qt/KDE friendly form.


class LogVol
{
    VolGroup *m_vg;
    QList<Segment *> m_segments;
    QList<MountEntry *> m_mount_entries;
    QList<LogVol *> m_lv_children;  // For a mirror the children are the legs and log
                                    // Snapshots are also children -- see m_snap_container
                                    // RAID Metadata is here too
    MountTables *m_tables;
    LogVol *m_lv_parent;       // NULL if this is the 'top' lv
    QString m_lv_full_name;    // volume_group/logical_volume
    QString m_lv_name;         // name of this logical volume
    QString m_lv_mapper_path;  // full path to volume, ie: /dev/vg1/lvol1
    QString m_lv_fs;         // Filesystem on volume, if known
    QString m_lv_fs_label;   // Filesystem label or name
    QString m_lv_fs_uuid;    // Filesystem uuid
    QString m_origin;        // the origin if this is a snapshot

    QString m_log;           // The mirror log, if this is a mirror
    QString m_type;          // the type of volume
    QString m_policy;        // the allocation policy
    QString m_state;         // the lv state

    QString m_uuid;
    QStringList m_tags;
    QString     m_fstab_mount_point;
    QStringList m_mount_points;  // empty if not mounted

    double  m_snap_percent;      // the percentage used, if this is a snapshot
    double  m_copy_percent;      // the percentage of extents moved, if pvmove underway
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
    bool m_virtual;              // virtual volume
    bool m_under_conversion;     // Is going to be a mirrored volume
    bool m_lvmmirror;            // Is an LVM (not raid 1) mirrored volume
    bool m_lvmmirror_leg;        // Is one of the underlying legs of a mirrored volume
    bool m_lvmmirror_log;        // Is the log for a mirrored volume
    bool m_metadata;             // Is RAID or thin pool metadata
    bool m_raid;                 // Is a raid volume, including raid 1 mirrors
    bool m_raid_image;           // Is a raid device under a raid volume
    bool m_fixed, m_persistent;  // fix the device minor and major number

    bool m_alloc_locked;         // allocation type is fixed when pvmove is underway
                                 // (and maybe other times)
    bool m_active;
    bool m_mounted;              // has a mounted filesystem
    bool m_open;                 // device is open
    bool m_orphan;               // virtual device with no pvs
    bool m_pvmove;               // is a pvmove temporary volume
    bool m_snap;                 // is a snapshot volume
    bool m_snap_container;       // is a fake lv that contains the real lv and its snapshots as children
    bool m_is_origin;
    bool m_writable;
    bool m_valid;                // is a valid snap
    bool m_merging;              // is snap or snap origin that is merging
    bool m_thin;                 // is thin

    void countLegsAndLogs();
    void processSegments(lv_t lvmLV, const QByteArray flags);
    QStringList removePvNames();     // list 'devices' that are really sub lvs
    QStringList getMetadataNames();  // names of sub lvs that are metadata for this lv
    QList<lv_t> getLvmSnapshots(vg_t lvmVG);
    void insertChildren(lv_t lvmLV, vg_t lvmVG);
    void calculateTotalSize();

public:
    LogVol(lv_t lvmLV, vg_t lvmVG, VolGroup *const vg, LogVol *const lvParent, MountTables *const tables, bool orphan = false);
    ~LogVol();

    void rescan(lv_t lvmLV, vg_t lvmVG);
    QList<LogVol *> getChildren();         // just the children -- not grandchildren etc.
    QList<LogVol *> takeChildren();        // removes the children from the logical volume
    QList<LogVol *> getAllChildrenFlat();  // All children, grandchildren etc. un-nested.
    QList<LogVol *> getSnapshots();        // This will work the same for snapcontainers or the real lv
    LogVol *getParent();                   // NULL if this is a "top level" lv
    VolGroup* getVg();
    QString getName();
    QString getFullName();
    QString getFilesystem();
    QString getFilesystemLabel();
    QString getFilesystemUuid();
    QString getMapperPath();
    QString getPolicy();
    QString getState();
    QString getType();
    int getRaidType();
    QString getOrigin();        // The name of the parent volume to a snapshot
    QString getUuid();
    int getSegmentCount();
    int getSegmentStripes(const int segment);     // The number of stipes including parity stripes
    int getSegmentStripeSize(const int segment);
    long long getSegmentSize(const int segment);
    long long getSegmentExtents(const int segment);
    QList<long long> getSegmentStartingExtent(const int segment);
    QStringList getPvNames(const int segment);
    QStringList getPvNamesAll();         // full path of physical volumes for all segments
    QStringList getPvNamesAllFlat();     // full path of physical volumes including child lvs, un-nested
    QStringList getMountPoints();
    QList<MountEntry *> getMountEntries();  // Calling function must delete these objects in the list
    QString getFstabMountPoint();
    QStringList getTags();
    long long getSpaceUsedOnPv(const QString physicalVolume);
    long long getExtents();
    long long getSize();
    long long getTotalSize();
    long long getFilesystemSize();
    long long getFilesystemUsed();
    double getSnapPercent();
    double getCopyPercent();
    unsigned long getMinorDevice();
    unsigned long getMajorDevice();
    int getLogCount();       // RAID 1 returns 0 since it doesn't have separate logs
    int getMirrorCount();
    int getSnapshotCount();
    bool isActive();
    bool isFixed();
    bool isLocked();
    bool isMerging();
    bool isMetadata();
    bool isLvmMirror();
    bool isLvmMirrorLeg();
    bool isLvmMirrorLog();
    bool isMounted();
    bool isOpen();
    bool isOrigin();
    bool isOrphan();
    bool isPersistent();
    bool isPvmove();
    bool isRaid();
    bool isRaidImage();
    bool isSnap();
    bool isSnapContainer();
    bool isThin();
    bool isUnderConversion();
    bool isValid();
    bool isVirtual();
    bool isWritable();
    bool hasMissingVolume();

};

#endif
