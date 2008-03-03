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


#include <QtGui>
#include "logvol.h"
#include "volgroup.h"


VolGroup::VolGroup(QString volumeGroupData)
{
    QString attributes;
    m_allocateable_extents = 0;
    
    volumeGroupData = volumeGroupData.trimmed();
    m_vg_name = volumeGroupData.section('|',0,0);
    m_pv_count = (volumeGroupData.section('|',1,1)).toInt();

/*  a bug(?) in lvm2 2.02 causes the reported logical volume count
    to be inconsistent with the number of logical volumes reported 
    by "lvs" when a physical volume move is in progress

    lv_count = (volumeGroupData.section('|',2,2)).toInt();  
*/
    m_lv_count = 0;
    m_snap_count = (volumeGroupData.section('|',3,3)).toInt();
    attributes   =  volumeGroupData.section('|',4,4);
    m_size       = (volumeGroupData.section('|',5,5)).toLongLong();
    m_free       = (volumeGroupData.section('|',6,6)).toLongLong();
    m_extent_size  = (volumeGroupData.section('|',7,7)).toLong();
    m_extents      = (volumeGroupData.section('|',8,8)).toLongLong();
    m_free_extents = (volumeGroupData.section('|',9,9)).toLongLong();
    m_lvm_fmt =  volumeGroupData.section('|',10,10);
    m_pv_max  = (volumeGroupData.section('|',11,11)).toInt();
    m_lv_max  = (volumeGroupData.section('|',12,12)).toInt();

    if(attributes.at(0) == 'w')
	m_writable = true;
    else 
	m_writable = false;

    if(attributes.at(1) == 'z')
	m_resizable = true;
    else
	m_resizable = false;

    if(attributes.at(2) == 'x')
	m_exported = true;
    else
	m_exported = false;

    if(attributes.at(3) == 'p')
	m_partial = true;
    else
	m_partial = false;

    if(attributes.at(4) == 'c')
	m_allocation_policy = "contiguous";
    else if(attributes.at(4) == 'l')
	m_allocation_policy = "cling";
    else if(attributes.at(4) == 'n')
	m_allocation_policy = "normal";
    else if(attributes.at(4) == 'a')
	m_allocation_policy = "anywhere";

    if(attributes.at(5) == 'c')
	m_clustered = true;
    else
	m_clustered = false;
}

/* The following function sorts some of the member volumes
   by type before returning the list. Any "Origin" volume 
   will be followed by all of its snapshots. Any volume that
   is not a snap or an origin for a snap gets put straight
   on the sorted list. If an origin is found it is appended to
   the sorted list and a loop starts that runs through the 
   unsorted list and finds all of its snapshots and appends 
   them to the sorted list after the origin. Finally if a
   snapshot turns up at the beginning of the unsorted list 
   before its origin, it gets appended back onto the end of 
   the unsorted list.  */

/* TO DO: a snapshot without an origin will cause an
   infinite loop! */

const QList<LogVol *>  VolGroup::getLogicalVolumes()
{
    QList<LogVol *> sorted_list;
    QList<LogVol *> unsorted_list = m_member_lvs;
    LogVol *lv;
    
    while( unsorted_list.size() ){
	lv = unsorted_list.takeFirst();
	if( lv->isOrigin() ){
	    sorted_list.append(lv);
	    for(int x = (unsorted_list.size() - 1); x >= 0; x--){
		if( unsorted_list[x]->isSnap() )
		    if( unsorted_list[x]->getOrigin() == lv->getName() )
			sorted_list.append( unsorted_list.takeAt(x) );
	    }
	}
	else if( lv->isSnap() )
	    unsorted_list.append( lv );     
	else                                 
	    sorted_list.append(lv);           
    }                                     
    return sorted_list;
}

const QList<PhysVol *> VolGroup::getPhysicalVolumes()
{
    return m_member_pvs;
}

LogVol* VolGroup::getLogVolByName(QString shortName)
{
    QString name;
    
    name = shortName.trimmed();
    for(int x = 0; x < m_lv_count; x++){
	if(name == m_member_lvs[x]->getName())
	    return m_member_lvs[x];
    }
    return NULL;
}

void VolGroup::addLogicalVolume(LogVol *logicalVolume)
{
    m_member_lvs.append(logicalVolume);
    m_lv_count = m_member_lvs.size();
}

void VolGroup::addPhysicalVolume(PhysVol *physicalVolume)
{
    m_member_pvs.append(physicalVolume);
    if( physicalVolume->isAllocateable() )
	m_allocateable_extents += physicalVolume->getUnused() / (long long) m_extent_size;
}

void VolGroup::clearPhysicalVolumes()
{
    m_member_pvs.clear();
}

long VolGroup::getExtentSize()
{
  return m_extent_size;
}

long long VolGroup::getExtents()
{
  return m_extents;
}

long long VolGroup::getFreeExtents()
{
  return m_free_extents;
}

long long VolGroup::getAllocateableExtents()
{
  return m_allocateable_extents;
}

long long VolGroup::getAllocateableSpace()
{
  return m_allocateable_extents * (long long)m_extent_size;
}

long long VolGroup::getSize()
{
    return m_extents * m_extent_size;
}

long long VolGroup::getFreeSpace()
{
  return m_free_extents * m_extent_size;
}

long long VolGroup::getUsedSpace()
{
  return (m_extents - m_free_extents) * m_extent_size;
}

int VolGroup::getLogVolCount()
{
  return m_lv_count;
}

int VolGroup::getLogVolMax()
{
  return m_lv_max;
}

int VolGroup::getPhysVolCount()
{
  return m_pv_count;
}

int VolGroup::getPhysVolMax()
{
  return m_pv_max;
}

int VolGroup::getSnapCount()
{
    return m_snap_count;
}

QString VolGroup::getName()
{
  return m_vg_name;
}

QString VolGroup::getPolicy()
{
    return m_allocation_policy;
}

QString VolGroup::getFormat()
{
  return m_lvm_fmt;
}

QStringList VolGroup::getLogVolNames()
{
  QStringList names;
  for(int x = 0; x < m_lv_count; x++){
    names << (m_member_lvs[x])->getName();
  }

  return names;
}

bool VolGroup::isWritable()
{
    return m_writable;
}

bool VolGroup::isResizable()
{
    return m_resizable;
}

bool VolGroup::isClustered()
{
    return m_clustered;
}

bool VolGroup::isPartial()
{
    return m_partial;
}

