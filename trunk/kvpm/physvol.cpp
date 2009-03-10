/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <QtGui>

#include "physvol.h"

PhysVol::PhysVol(QString pvData)
{
    QString attributes;

    m_extent_size = 0;
    m_last_used_extent = 0;

    pvData     = pvData.trimmed();
    m_device   = pvData.section('|',0,0);
    m_vg_name  = pvData.section('|',1,1);
    m_format   = pvData.section('|',2,2);
    attributes = pvData.section('|',3,3);

    if (attributes.at(0) == 'a')
	m_allocatable = true;
    else
	m_allocatable = false;

    if (attributes.at(1) == 'x')
	m_exported = true;
    else
	m_exported = false;

    m_size   = (pvData.section('|',4,4)).toLongLong();
    m_unused = (pvData.section('|',5,5)).toLongLong();
    m_used   = (pvData.section('|',6,6)).toLongLong();
    m_uuid   =  pvData.section('|',7,7);
}

QString PhysVol::getVolumeGroupName()
{
    return m_vg_name;
}

QString PhysVol::getDeviceName()
{
    return m_device;
}

QString PhysVol::getFormat()
{
    return m_format;
}

QString PhysVol::getUuid()
{
    return m_uuid;
}

bool PhysVol::isAllocateable()
{
    return m_allocatable;
}

bool PhysVol::isExported()
{
    return m_exported;
}

long long PhysVol::getSize()
{
    return m_size;
}

long long PhysVol::getUnused()
{
    return m_unused;
}

long long PhysVol::getUsed()
{
    return m_used;
}

int PhysVol::getPercentUsed()
{
    int percent;

    if( m_used == m_size )
	return 100;
    else if( m_used == 0 )
	return 0;
    else if( m_size == 0 )      // This shouldn't happen
	return 100;
    else
	percent = qRound(  ( m_used * 100.0 ) / m_size );
    
    return percent;
}

long long PhysVol::getLastUsedExtent()
{
    return m_last_used_extent;
}

void PhysVol::setLastUsedExtent(long long last)
{
    m_last_used_extent = last;
}

long PhysVol::getExtentSize()
{
    return m_extent_size;
}

void PhysVol::setExtentSize(long size)
{
    m_extent_size = size;
}
