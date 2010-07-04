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

#ifndef PHYSVOL_H
#define PHYSVOL_H

#include <lvm2app.h>

#include <QString>
#include <QWidget>

class VolGroup;

class PhysVol
{
    QString m_device;      // eg: /dev/hde4
    QString m_format;      // e.g. lvm1 or lvm2
    QString m_uuid;
    VolGroup *m_vg;        // all pvs now must be in a vg
    bool m_active;
    bool m_allocatable;
    bool m_exported;
    long long m_size;         // size in bytes of physical volume
    long long m_device_size;  // size in bytes of underlying device
    long long m_unused;       // free space in bytes
    long long m_last_used_extent;

 public:
    PhysVol(pv_t pv, VolGroup *vg);
    QString getDeviceName();       // eg: /dev/hde4
    QString getFormat();           // e.g. lvm1 or lvm2
    QString getUuid();
    VolGroup* getVolGroup(); 
    bool isAllocateable();
    bool isExported();
    void setActive(bool active);
    bool isActive();
    long long getSize();            // size of the physical volume in bytes    
    long long getDeviceSize();      // the physical volume might not take up all the device!    
    long long getUnused();          // free space in bytes
    long long getLastUsedExtent();  // needed for minimum shrink size determination
    void setLastUsedExtent(long long last);
    int getPercentUsed();           // 0 - 100
    
};

#endif
