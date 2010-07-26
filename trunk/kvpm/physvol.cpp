/*
 *
 * 
 * Copyright (C) 2008, 2010 Benjamin Scott   <benscott@nwlink.com>
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
    rescan(pv);
}

void PhysVol::rescan(pv_t pv)
{
    m_last_used_extent = 0;

    m_device   = QString( lvm_pv_get_name(pv) );
    m_allocatable = true; // Set this correctly when lvm2apps ready!!!

    // pv is active if any associated lvs are active
    m_active = false;
    
    m_device_size   = lvm_pv_get_dev_size(pv); 
    m_unused        = lvm_pv_get_free(pv);
    m_size          = lvm_pv_get_size(pv);
    m_uuid          = QString( lvm_pv_get_uuid(pv) );

    return;
}

VolGroup* PhysVol::getVolGroup()
{
    return m_vg;
}

QString PhysVol::getDeviceName()
{
    return m_device.trimmed();
}

QString PhysVol::getUuid()
{
    return m_uuid.trimmed();
}

bool PhysVol::isAllocateable()
{
    return m_allocatable;
}

bool PhysVol::isActive()
{
    return m_active;
}

void PhysVol::setActive()
{
    m_active = true;
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
