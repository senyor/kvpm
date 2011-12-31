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


#ifndef VOLGROUP_H
#define VOLGROUP_H

#include <lvm2app.h>

#include <QWidget>
#include <QStringList>

class LogVol;
class MountTables;
class PhysVol;

class VolGroup
{
    MountTables *m_tables;

    long m_extent_size;
    int m_lv_max;          // maximum number of logical volumes
    int m_pv_max;
    long long m_mda_count;
    long long m_size;      // total size of volume group in bytes
    long long m_free;      // free space in bytes
    long long m_extents;
    long long m_free_extents;         // free extents are not always useable
    long long m_allocatable_extents; // extents on some physical volumes
                                      // may not be allocateable
    QString m_vg_name;                // this volume group name
    QString m_uuid;
    QString m_allocation_policy;
    QString m_lvm_format;                // lvm1 or lvm2
    QList<LogVol *>  m_member_lvs;    // lvs that belong to this group
    QList<PhysVol *> m_member_pvs;    // pvs that belong to this group
    bool m_active;         // if any lv is active the group is active
    bool m_writable;
    bool m_resizable;
    bool m_clustered;
    bool m_exported;
    bool m_partial;        // some physical volumes may be missing

    lv_t findOrphan(QList<lv_t> &childList);
    void processLogicalVolumes(vg_t lvmVG);
    void processPhysicalVolumes(vg_t lvmVG);
    void setActivePhysicalVolumes();
    void setLastUsedExtent();

public:
    VolGroup(lvm_t lvm, const char *vgname, MountTables *const tables);
    ~VolGroup();
    void rescan(lvm_t lvm);
    QList<LogVol *>  getLogicalVolumes();     // *TOP LEVEL ONLY* snapcontainers returned not snaps and origin 
    QList<LogVol *>  getLogicalVolumesFlat(); // un-nest the volumes, snapshots and mirror legs
    QList<PhysVol *> getPhysicalVolumes();
    LogVol* getLvByName(QString shortName);   // lv name without the vg name and "/" -- skips snap containers
    LogVol* getLvByUuid(QString uuid);        // also skips snap containers
    PhysVol* getPvByName(QString name);       //   /dev/something
    long long getExtents();
    long long getFreeExtents();
    long long getAllocatableExtents();
    long long getAllocatableSpace();
    long getExtentSize();
    long long getSize();
    long long getFreeSpace();
    long long getUsedSpace();
    int getLvCount();
    int getLvMax();
    int getPvCount();
    int getPvMax();
    int getMdaCount();
    QString getName();
    QString getUuid();
    QString getPolicy();
    QString getFormat();
    QStringList getLvNames();
    bool isWritable();
    bool isResizable();
    bool isClustered();
    bool isPartial();
    bool isExported();
    bool isActive();
    
};

#endif
