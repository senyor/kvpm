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
#ifndef LOGVOL_H
#define LOGVOL_H

#include <QWidget>
#include <QStringList>
#include "volgroup.h"

class MountInformation;
class MountInformationList;

struct Segment
{
    int stripes, stripe_size;    // number of stripes in segment
    long long size;              // segment size (bytes)
    QStringList device_path;     // full path of physical volume
    QList<long long> starting_extent;   // first extent on physical volume for this segment
};

class LogVol
{
    VolGroup *group;
    QList<Segment *> segments;
    QList<MountInformation *> mount_info_list;
    
    QString full_name;            // volume_group/logical_volume
    QString lv_name;              // name of this logical volume
    QString lv_fs;                // Filesystem on volume or "unknown"
    QString snap_origin;          // the origin if this is a snapshot 
    QString type, policy, state;  // the type of volume, allocation policy and state
    QString vg_name;              // associated volume group name
    QStringList mount_points;     // empty if not mounted

    double  snap_percent;         // the percentage used, if this is a snapshot
    double  copy_percent;         // the percentage of extents moved, if pvmove underway
    long long size;               // size in bytes
    long long extents;            // size in extents
    int seg_total;                // total number of segments in logical volume

    int major_device, minor_device; // Unix device major and minor number, if set
    bool fixed, persistant;    // fix the device minor and major number 

    bool alloc_locked;         // allocation type is fixed when pvmove is underway 
                               // (and maybe other times)
    bool mounted;
    bool open;                 // device is open
    bool pvmove;               // is a pvmove temporary volume
    bool snap;                 // is a snapshot volume
    bool writable;

    Segment* processSegments(QString segment_data);
    
 public:
    LogVol(QStringList lvdata_list, MountInformationList *mount_list);
    QString getVolumeGroupName();
    QString getName();
    QString getFullName();
    QString getFS();
    const QString getMapperPath();
    QString getPolicy();
    QString getState();
    QString getType();
    QString getOrigin();
    int getSegmentCount();
    int getSegmentStripes(int segment);
    int getSegmentStripeSize(int segment);
    long long getSegmentSize(int segment);
    long long getSegmentExtents(int segment);
    QList<long long> getSegmentStartingExtent(int segment);
    QStringList getDevicePath(int segment);
    QStringList getDevicePathAll();
    QStringList getMountPoints();
    long long getSpaceOnPhysicalVolume(QString PhysicalVolume);
    long long getExtents();
    long long getSize();
    VolGroup* getVolumeGroup();
    void setVolumeGroup(VolGroup *VolumeGroup);
    double getSnapPercent();
    double getCopyPercent();
    int getMinorDevice();
    int getMajorDevice();
    bool isFixed();
    bool isLocked();
    bool isMounted();
    bool isOpen();
    bool isOrigin();
    bool isPersistant();
    bool isPvmove();
    bool isSnap();
    bool isWritable();
    
};

#endif
