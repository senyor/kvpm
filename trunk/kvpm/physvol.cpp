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
#include "volgroup.h"

PhysVol::PhysVol(pv_t pv, VolGroup *vg)
{
    m_vg = vg;
    m_last_used_extent = 0;

    m_device   = QString( lvm_pv_get_name(pv) );
    m_format   = "????"; // Set these when lvm2apps ready!!!

    m_allocatable = true; // Set these when lvm2apps ready!!!

    // pv is active if any associated lvs are active
    m_active   = true;    // Set these when lvm2apps ready!!!
    m_exported = false;   // Set these when lvm2apps ready!!!
    
    m_device_size   = lvm_pv_get_dev_size(pv); 
    m_unused        = lvm_pv_get_free(pv);
    m_size          = lvm_pv_get_size(pv);
    m_uuid          = QString( lvm_pv_get_uuid(pv) );

    qDebug("PV Dev Size %lld  Used %lld  Unused %lld", m_device_size, m_size, m_unused);
}

VolGroup* PhysVol::getVolGroup()
{
    return m_vg;
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

bool PhysVol::isActive()
{
    return m_active;
}

void PhysVol::setActive(bool active)
{
    m_active = active;
}

long long PhysVol::getSize()
{
    return m_size;
}

long long PhysVol::getDeviceSize()
{
    return m_device_size;
}

long long PhysVol::getUnused()
{
    return m_unused;
}

int PhysVol::getPercentUsed()
{
    int percent;

    if( m_unused == 0 )
	return 100;
    else if( m_unused == m_size )
	return 0;
    else if( m_size == 0 )      // This shouldn't happen
	return 100;
    else
	percent = qRound(  ( ( m_size - m_unused ) * 100.0 ) / m_size );
    
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
