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


#ifndef VOLGROUP_H
#define VOLGROUP_H

#include <lvm2app.h>

#include <QStringList>

#include "allocationpolicy.h"
#include "logvol.h"
#include "misc.h"

class QWidget;

class LogVol;
class MountTables;
class PhysVol;

class VolGroup
{
    MountTables *m_tables;

    uint64_t m_extent_size;
    int m_lv_max;          // maximum number of logical volumes, unlimited == 0
    int m_pv_max;
    long long m_mda_count;
    long long m_size;      // total size of volume group in bytes
    long long m_free;      // free space in bytes
    long long m_extents;
    long long m_free_extents;         // free extents are not always useable
    long long m_allocatable_extents;  // extents on some physical volumes
                                      // may not be allocateable
    QString m_vg_name;                // this volume group name
    QString m_uuid;
    QString m_lvm_format;             // lvm1 or lvm2
    AllocationPolicy m_policy;
    LogVolList m_member_lvs;        // lvs that belong to this group
    QList<PhysVol *> m_member_pvs;  // pvs that belong to this group
    QStringList m_lv_names_all;     // names of all lvs and sub lvs in group, including metadata
    bool m_active;         // if any lv is active the group is active
    bool m_writable;
    bool m_resizable;
    bool m_clustered;
    bool m_exported;
    bool m_partial;        // some physical volumes may be missing
    bool m_open_failed;    // lvm couldn't open the vg, clustering without a working clvmd can do that

    lv_t findOrphan(QList<lv_t> &childList);
    void processLogicalVolumes(vg_t lvmVG);
    void processPhysicalVolumes(vg_t lvmVG);
    void setActivePhysicalVolumes();
    void setLastUsedExtent();

public:
    VolGroup(lvm_t lvm, const char *vgname, MountTables *const tables);
    ~VolGroup();
    void rescan(lvm_t lvm);
    LogVolList getLogicalVolumes();        // *TOP LEVEL ONLY* snapcontainers returned not snaps and origin
    LogVolList getLogicalVolumesFlat();    // un-nest the volumes, snapshots and mirror legs
    QList<PhysVol *> getPhysicalVolumes();
    QStringList getLvNamesAll();                  // unsorted list of all lvs and sub lvs
    LogVolPointer getLvByName(QString shortName); // lv name without the vg name and "/" -- skips snap containers
    LogVolPointer getLvByUuid(QString uuid);      // also skips snap containers
    PhysVol* getPvByName(QString name);           //   /dev/something
    long long getExtents();
    long long getFreeExtents();
    long long getAllocatableExtents();
    long long getAllocatableSpace();
    uint64_t getExtentSize();
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
    QString getFormat();
    AllocationPolicy getPolicy();
    QStringList getLvNames();
    bool isWritable();
    bool isResizable();
    bool isClustered();
    bool isPartial();
    bool isExported();
    bool isActive();
    bool openFailed();

};

#endif
