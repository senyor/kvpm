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

#ifndef PHYSVOL_H
#define PHYSVOL_H

#include <lvm2app.h>

#include <QStringList>

class VolGroup;
class LogVol;

struct LVSegmentExtent {
    QString lv_name;
    long long first_extent;
    long long last_extent;
};


class PhysVol
{
    QString m_device;         // eg: /dev/hde4
    QString m_mapper_device;  // eg: /dev/mapper/foo if it is dmraid, equals m_device otherwise
    QString m_format;         // e.g. lvm1 or lvm2
    QString m_uuid;
    QStringList m_tags;
    VolGroup *m_vg;           // all pvs now must be in a vg
    bool m_active;
    bool m_allocatable;
    bool m_missing;           // the physical volume can't be found
    uint64_t  m_mda_count;    // number of metadata areas
    uint64_t  m_mda_used;     // number of metadata areas in use
    uint64_t  m_mda_size;
    long long m_size;         // size in bytes of physical volume
    long long m_device_size;  // size in bytes of underlying device
    long long m_unused;       // free space in bytes
    long long m_last_used_extent;

    QString findMapperPath(QString path);

public:
    PhysVol(pv_t lvm_pv, VolGroup *const vg);
    void rescan(pv_t pv);
    QString getName() const { return m_device.trimmed(); }       // eg: /dev/hde4
    QString getMapperName() const { return m_mapper_device.trimmed(); }    
    QString getUuid() const { return m_uuid.trimmed(); } 
    QStringList getTags() const { return m_tags; }
    VolGroup* getVg() const { return m_vg; }
    bool isAllocatable() const { return m_allocatable; }
    void setActive() { m_active = true; }        // If any lv is active on the pv, the pv is active
    bool isActive() const { return m_active; }
    bool isMissing() const { return m_missing; }
    long long getContiguous(LogVol *lv);   // the number of contiguous bytes available if the lv is on this pv
    long long getContiguous();             // the max contiguous bytes on the the pv.
    long long getSize() const { return m_size; }              // size of the physical volume in bytes
    long long getDeviceSize() const { return m_device_size; } // the physical volume might not take up all the device!
    long long getRemaining() const { return m_unused; }      // free space in bytes
    long long getLastUsedExtent() const { return m_last_used_extent; }    // needed for minimum shrink size determination
    void setLastUsedExtent(const long long last) { m_last_used_extent = last; }
    int getPercentUsed();           // 0 - 100
    long getMdaCount() const { return m_mda_count; }
    long getMdaUsed() const { return m_mda_used; }           // Meta Data areas in use
    long long getMdaSize() const { return m_mda_size; }      // Meta Data Area size in bytes
    QList<LVSegmentExtent *> sortByExtent();
};

#endif
