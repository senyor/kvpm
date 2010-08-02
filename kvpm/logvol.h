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

#ifndef LOGVOL_H
#define LOGVOL_H

#include <QWidget>
#include <QStringList>

class MountInformation;
class MountInformationList;
class VolGroup;
class Segment;

class LogVol
{
    VolGroup *m_group;
    QList<Segment *> m_segments;
    QList<MountInformation *> m_mount_info_list;
    
    QString m_lv_full_name;  // volume_group/logical_volume
    QString m_lv_name;       // name of this logical volume
    QString m_lv_fs;         // Filesystem on volume or "unknown"
    QString m_lvm_format;    // lvm1 or lvm2
    QString m_vg_attr;       // vgs vg_attr string
    QString m_origin;        // the origin if this is a snapshot or 
                             // the parent mirror volume to a mirror leg

    QString m_type;          // the type of volume
    QString m_policy;        // the allocation policy
    QString m_state;         // the lv state
    QString m_vg_name;       // associated volume group name

    QString m_uuid;
    QStringList m_tags;
    QStringList m_mount_points;  // empty if not mounted
    QList<int> m_mount_position; // if mountpoint has multiple mounts 
                                 // position 1 is the most recently mounted one.

    double  m_snap_percent;      // the percentage used, if this is a snapshot
    double  m_copy_percent;      // the percentage of extents moved, if pvmove underway
    long long m_size;            // size in bytes
    long long m_extents;         // size in extents
    int m_seg_total;             // total number of segments in logical volume
    int m_major_device;          // Unix device major number, if set
    int m_minor_device;          // Unix device minor number, if set
    int m_log_count;             // if a mirror -- how many logs
    bool m_virtual;              // virtual volume
    bool m_under_conversion;     // Is going to be a mirrored volume
    bool m_mirror;               // Is a mirrored volume
    bool m_mirror_leg;           // Is one of the underlying legs of a mirrored volume
    bool m_mirror_log;           // Is the log for a mirrored volume
    bool m_fixed, m_persistant;  // fix the device minor and major number 

    bool m_alloc_locked;         // allocation type is fixed when pvmove is underway 
                                 // (and maybe other times)
    bool m_active;
    bool m_mounted;              // has a mounted filesystem
    bool m_open;                 // device is open
    bool m_orphan;               // virtual device with no pvs
    bool m_pvmove;               // is a pvmove temporary volume
    bool m_snap;                 // is a snapshot volume
    bool m_writable;

    Segment* processSegments(QString segmentData);
    
 public:
    LogVol(QStringList lvDataList, MountInformationList *mountInformationList);
    QString getVolumeGroupName();
    QString getName();
    QString getFullName();
    QString getFilesystem();
    QString getMapperPath();
    QString getPolicy();
    QString getState();
    QString getType();
    QString getOrigin();        // The name of the parent volume to a snapshot or mirror leg
    void setOrigin(QString origin);
    QString getUuid();
    QString getLVMFormat();
    QString getVGAttr();
    int getSegmentCount();
    int getSegmentStripes(int segment);
    int getSegmentStripeSize(int segment);
    long long getSegmentSize(int segment);
    long long getSegmentExtents(int segment);
    QList<long long> getSegmentStartingExtent(int segment);
    QStringList getDevicePath(int segment);     
    QStringList getDevicePathAll();         // full path of physical volumes for all segments
    QStringList getMountPoints();
    QStringList getTags();
    QList<int>  getMountPosition();
    long long getSpaceOnPhysicalVolume(QString physicalVolume);
    long long getExtents();
    long long getSize();
    VolGroup* getVolumeGroup();
    void setVolumeGroup(VolGroup *volumeGroup);
    double getSnapPercent();
    double getCopyPercent();
    int getMinorDevice();
    int getMajorDevice();
    int getLogCount();
    void setLogCount(int count);
    bool isActive();
    bool isFixed();
    bool isLocked();
    bool isMirror();
    bool isMirrorLeg();
    bool isMirrorLog();
    bool isMounted();
    bool isOpen();
    bool isOrigin();
    bool isOrphan();
    bool isPersistant();
    bool isPvmove();
    bool isSnap();
    bool isUnderConversion();
    bool isVirtual();
    bool isWritable();
    
};

#endif
