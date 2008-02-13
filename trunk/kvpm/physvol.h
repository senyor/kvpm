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
#ifndef PHYSVOL_H
#define PHYSVOL_H
#include <QString>
#include <QWidget>

class PhysVol
{
    QString device;      // eg: /dev/hde4
    QString vg_name;     // May be empty, if it isn't assigned to a group yet
    QString format;      // e.g. lvm1 or lvm2
    QString uuid;
    
    bool allocatable;
    bool exported;
    long long size;      // size in bytes
    long long unused;      // free space in bytes
    long long used;

 public:
    PhysVol(QString lvdata);
    QString getDeviceName();
    QString getVolumeGroupName();
    QString getFormat();
    QString getUuid();
    bool isAllocateable();
    bool isExported();
    long long getSize();
    long long getUnused();
    long long getUsed();
    int getPercentUsed();
    
};

#endif
