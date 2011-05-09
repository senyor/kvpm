/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
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

void PhysVol::rescan(pv_t lvm_pv)
{
    QByteArray flags;
    lvm_property_value value;

    value = lvm_pv_get_property(lvm_pv, "pv_attr");
    flags.append(value.value.string);

    if(flags[0] == 'a')
        m_allocatable = true;
    else
        m_allocatable = false;

    m_last_used_extent = 0;
    m_active        = false;     // pv is active if any associated lvs are active
    m_device        = QString( lvm_pv_get_name(lvm_pv) );
    m_device_size   = lvm_pv_get_dev_size(lvm_pv); 
    m_unused        = lvm_pv_get_free(lvm_pv);
    m_size          = lvm_pv_get_size(lvm_pv);
    m_uuid          = QString( lvm_pv_get_uuid(lvm_pv) );
    m_mda_count     = lvm_pv_get_mda_count(lvm_pv);

    /*
    // The following wil be used to to calculate the last used
    // segement once the "lv_name" property gets implemented
    dm_list* pvseg_dm_list = lvm_pv_list_pvsegs(lvm_pv);  
    lvm_pvseg_list *pvseg_list;

    if(pvseg_dm_list){
        dm_list_iterate_items(pvseg_list, pvseg_dm_list){ 
	    
            //            value = lvm_pvseg_get_property( pvseg_list->pvseg , "lv_name");
            value = lvm_pvseg_get_property( pvseg_list->pvseg , "pvseg_start");
            if(value.is_valid)
                qDebug() << "Seg start: " << value.value.integer;
            else
                qDebug() << "Not valid";

            value = lvm_pvseg_get_property( pvseg_list->pvseg , "pvseg_size");
            if(value.is_valid)
                qDebug() << "Seg size:  " << value.value.integer;
            else
                qDebug() << "Not valid";
	}
    }
    */
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

long long PhysVol::getMDACount()
{
    return m_mda_count;
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
