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
#ifndef VOLGROUP_H
#define VOLGROUP_H

#include <QWidget>
#include <QStringList>
#include "physvol.h"

class LogVol;

class VolGroup
{
    long extent_size;
    int pv_count;
    int pv_max;
    long long size;      // total size of volume group in bytes
    long long free;      // free space in bytes
    long long extents;
    long long free_extents;         // free extents are not always useable
    long long allocateable_extents; // extents on some physical volumes
                                    // may not be allocateable
    int lv_count;        // number of logical volumes in this volume group
    int lv_max;          // maximum number
    int snap_count;      // snapshots count
    QString vg_name;     // this volume group name
    QString allocation_policy;
    QString lvm_fmt;     // lvm1 or lvm2
    QList<LogVol *>  member_lvs;
    QList<PhysVol *> member_pvs;
    bool resizable;
    bool clustered;
    bool writable;
    bool exported;
    bool partial;
    
public:
    VolGroup(QString vgdata);
    void addLogicalVolume(LogVol *lv);
    void addPhysicalVolume(PhysVol *pv);
    void clearPhysicalVolumes();
    const QList<LogVol *>  getLogicalVolumes();
    const QList<PhysVol *> getPhysicalVolumes();
    LogVol* getLogVolByName(QString ShortName);  // lv name with the vg name and "/"
    long long getExtents();
    long long getFreeExtents();
    long long getAllocateableExtents();
    long long getAllocateableSpace();
    long getExtentSize();
    long long getSize();
    long long getFreeSpace();
    long long getUsedSpace();
    int getLogVolCount();
    int getLogVolMax();
    int getPhysVolCount();
    int getPhysVolMax();
    int getSnapCount();
    QString getName();
    QString getPolicy();
    QString getFormat();
    QStringList getLogVolNames();
    bool isResizable();
    bool isWritable();
    bool isClustered();
    bool isPartial();
    
};

#endif
