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


#include <QtGui>
#include "physvol.h"

PhysVol::PhysVol(QString pvdata)
{
    QString attr;
    allocatable = FALSE;
    exported = FALSE;
    
    pvdata  = pvdata.trimmed();
    device  = pvdata.section('|',0,0);
    vg_name = pvdata.section('|',1,1);
    format  = pvdata.section('|',2,2);
    attr    = pvdata.section('|',3,3);
    if (attr.at(0) == 'a')
	allocatable = TRUE;
    if (attr.at(1) == 'x')
	exported = TRUE;
    size    = (pvdata.section('|',4,4)).toLongLong();
    unused  = (pvdata.section('|',5,5)).toLongLong();
    used  = (pvdata.section('|',6,6)).toLongLong();
    uuid = pvdata.section('|',7,7);
}

QString PhysVol::getVolumeGroupName()
{
    return vg_name;
}

QString PhysVol::getDeviceName()
{
    return device;
}

QString PhysVol::getFormat()
{
    return format;
}

QString PhysVol::getUuid()
{
    return uuid;
}

bool PhysVol::isAllocateable()
{
    return allocatable;
}

bool PhysVol::isExported()
{
    return exported;
}

long long PhysVol::getSize()
{
    return size;
}

long long PhysVol::getUnused()
{
    return unused;
}

long long PhysVol::getUsed()
{
    return used;
}

int PhysVol::getPercentUsed()
{
    int percent;

    if( used == size )
	return 100;
    else if( used == 0 )
	return 0;
    else if( size == 0 )      // This shouldn't happen
	return 100;
    else
	percent = qRound(  ( used * 100.0 ) / size );
    
    
    return percent;
}
