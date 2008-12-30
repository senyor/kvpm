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

#include <QString>
#include <QWidget>

class PhysVol
{
    QString m_device;      // eg: /dev/hde4
    QString m_vg_name;     // May be empty, if it isn't assigned to a group yet
    QString m_format;      // e.g. lvm1 or lvm2
    QString m_uuid;
    
    bool m_allocatable;
    bool m_exported;
    long long m_size;      // size in bytes
    long long m_unused;    // free space in bytes
    long long m_used;

 public:
    PhysVol(QString pvData);
    QString getDeviceName();       // eg: /dev/hde4
    QString getVolumeGroupName();
    QString getFormat();           // e.g. lvm1 or lvm2
    QString getUuid();
    bool isAllocateable();
    bool isExported();
    long long getSize();       // size of the physical volume in bytes    
    long long getUnused();     // free space in bytes
    long long getUsed();       // bytes used 
    int getPercentUsed();      // 0 - 100
    
};

#endif
