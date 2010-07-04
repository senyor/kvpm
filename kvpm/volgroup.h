/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef VOLGROUP_H
#define VOLGROUP_H

#include <lvm2app.h>

#include <QWidget>
#include <QStringList>

class PhysVol;
class LogVol;

class VolGroup
{
    long m_extent_size;
    int m_lv_max;          // maximum number of logical volumes
    int m_pv_max;
    long long m_size;      // total size of volume group in bytes
    long long m_free;      // free space in bytes
    long long m_extents;
    long long m_free_extents;         // free extents are not always useable
    long long m_allocateable_extents; // extents on some physical volumes
                                      // may not be allocateable
    QString m_vg_name;                // this volume group name
    QString m_allocation_policy;
    QString m_lvm_fmt;                // lvm1 or lvm2
    QList<LogVol *>  m_member_lvs;    // lvs that belong to this group
    QList<PhysVol *> m_member_pvs;    // pvs that belong to this group
    bool m_resizable;
    bool m_clustered;
    bool m_writable;
    bool m_exported;
    bool m_partial;        // some physical volumes may be missing
    
public:
    VolGroup(lvm_t lvm, const char *vgname);
    void addLogicalVolume(LogVol *logicalVolume);
    const QList<LogVol *>  getLogicalVolumes();
    const QList<PhysVol *> getPhysicalVolumes();
    LogVol* getLogVolByName(QString shortName);  // lv name without the vg name and "/"
    PhysVol* getPhysVolByName(QString name);
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
    QString getName();
    QString getPolicy();
    QString getFormat();
    QStringList getLogVolNames();
    bool isResizable();
    bool isWritable();
    bool isClustered();
    bool isPartial();
    bool isExported();
    
};

#endif
