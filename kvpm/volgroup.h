/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013, 2014 Benjamin Scott   <benscott@nwlink.com>
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
    LvList m_member_lvs;        // lvs that belong to this group
    QList<PhysVol *> m_member_pvs;  // pvs that belong to this group

    long long m_extent_size;
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
    LvList getLogicalVolumes() const { return m_member_lvs; } // *TOP LEVEL ONLY* snapcontainers returned not snaps and origin
    LvList getLogicalVolumesFlat() const;    // un-nest the volumes, snapshots and mirror legs
    QList<PhysVol *> getPhysicalVolumes() const { return m_member_pvs; }
    QStringList getLvNamesAll() const { return m_lv_names_all; } // unsorted list of all lvs and sub lvs
    LvPtr getLvByName(QString shortName) const; // lv name without the vg name and "/" -- skips snap containers
    LvPtr getLvByUuid(QString uuid) const;      // also skips snap containers
    PhysVol* getPvByName(QString name) const;           //   /dev/something
    long long getExtents() const { return m_extents; }
    long long getFreeExtents() const { return m_free_extents; }
    long long getAllocatableExtents() const { return m_allocatable_extents; }
    long long getAllocatableSpace() const { return m_allocatable_extents * (long long)m_extent_size; }
    long long getExtentSize() const { return m_extent_size; }
    long long getSize() const { return m_extents * m_extent_size; }
    long long getFreeSpace() const { return m_free_extents * m_extent_size; }
    long long getUsedSpace() const { return (m_extents - m_free_extents) * m_extent_size; }
    int getLvCount() const { return m_member_lvs.size(); }
    int getLvMax() const { return m_lv_max; } 
    int getPvCount() const { return m_member_pvs.size(); }
    int getPvMax() const { return m_pv_max; }
    int getMdaCount() const { return m_mda_count; }
    QString getName() const { return m_vg_name; }
    QString getUuid() const { return m_uuid; } 
    QString getFormat() const { return m_lvm_format; }
    AllocationPolicy getPolicy() const { return m_policy; }
    QStringList getLvNames() const; 
    bool isWritable() const { return m_writable; }
    bool isResizable() const { return m_resizable; }
    bool isClustered() const { return m_clustered; }
    bool isPartial() const { return m_partial; }
    bool isExported() const { return m_exported; }
    bool isActive() const { return m_active; } 
    bool openFailed() const { return m_open_failed; }

};

#endif
