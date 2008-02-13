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
#include "logvol.h"
#include "volgroup.h"


VolGroup::VolGroup(QString vgdata)
{
    QString attributes;
    allocateable_extents = 0;
    
    vgdata = vgdata.trimmed();
    vg_name = vgdata.section('|',0,0);
    pv_count = (vgdata.section('|',1,1)).toInt();

/*  a bug in lvm2 2.02 causes the reported logical volume count
    to be inconsistent with the number of logical volumes reported 
    by /sbin/lvs when a physical volume move is in progress

    lv_count = (vgdata.section('|',2,2)).toInt();  
*/
    lv_count = 0;
    snap_count = (vgdata.section('|',3,3)).toInt();
    attributes = vgdata.section('|',4,4);
    size = (vgdata.section('|',5,5)).toLongLong();
    free = (vgdata.section('|',6,6)).toLongLong();
    extent_size = (vgdata.section('|',7,7)).toLong();
    extents = (vgdata.section('|',8,8)).toLongLong();
    free_extents = (vgdata.section('|',9,9)).toLongLong();
    lvm_fmt = vgdata.section('|',10,10);
    pv_max = (vgdata.section('|',11,11)).toInt();
    lv_max = (vgdata.section('|',12,12)).toInt();
    if(attributes.at(0) == 'w')
	writable = TRUE;
    else 
	writable = FALSE;
    if(attributes.at(1) == 'z')
	resizable = TRUE;
    else
	resizable = FALSE;
    if(attributes.at(2) == 'x')
	exported = TRUE;
    else
	exported = FALSE;
    if(attributes.at(3) == 'p')
	partial = TRUE;
    else
	partial = FALSE;
    if(attributes.at(4) == 'c')
	allocation_policy = "contiguous";
    else if(attributes.at(4) == 'l')
	allocation_policy = "cling";
    else if(attributes.at(4) == 'n')
	allocation_policy = "normal";
    else if(attributes.at(4) == 'a')
	allocation_policy = "anywhere";
    if(attributes.at(5) == 'c')
	clustered = TRUE;
    else
	clustered = FALSE;
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
    QList<LogVol *> sorted_list, unsorted_list;
    LogVol *lv;

    unsorted_list = member_lvs;
    
    while(unsorted_list.size()){
	lv = unsorted_list.takeFirst();
	if( lv->isOrigin() ){
	    sorted_list.append(lv);
	    for(int x = (unsorted_list.size() - 1); x >= 0; x--){
		if( unsorted_list[x]->isSnap() )
		    if(unsorted_list[x]->getOrigin() == lv->getName())
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
    return member_pvs;
}

LogVol* VolGroup::getLogVolByName(QString ShortName)
{
    QString name;
    
    name = ShortName.trimmed();
    for(int x = 0; x < lv_count; x++){
	if(name == member_lvs[x]->getName())
	    return member_lvs[x];
    }
    return NULL;
}


void VolGroup::addLogicalVolume(LogVol *lv)
{
    member_lvs.append(lv);
    lv_count = member_lvs.size();
}

void VolGroup::addPhysicalVolume(PhysVol *pv)
{
    member_pvs.append(pv);
    if( pv->isAllocateable() )
	allocateable_extents += pv->getUnused() / (long long) extent_size;
}

void VolGroup::clearPhysicalVolumes()
{
    member_pvs.clear();
}

long VolGroup::getExtentSize()
{
  return extent_size;
}

long long VolGroup::getExtents()
{
  return extents;
}

long long VolGroup::getFreeExtents()
{
  return free_extents;
}

long long VolGroup::getAllocateableExtents()
{
  return allocateable_extents;
}

long long VolGroup::getAllocateableSpace()
{
  return allocateable_extents * (long long)extent_size;
}

long long VolGroup::getSize()
{
    return extents * extent_size;
}

long long VolGroup::getFreeSpace()
{
  return free_extents * extent_size;
}

long long VolGroup::getUsedSpace()
{
  return (extents - free_extents) * extent_size;
}

int VolGroup::getLogVolCount()
{
  return lv_count;
}

int VolGroup::getLogVolMax()
{
  return lv_max;
}

int VolGroup::getPhysVolCount()
{
  return pv_count;
}

int VolGroup::getPhysVolMax()
{
  return pv_max;
}

int VolGroup::getSnapCount()
{
    return snap_count;
}

QString VolGroup::getName()
{
  return vg_name;
}

QString VolGroup::getPolicy()
{
    return allocation_policy;
}

QString VolGroup::getFormat()
{
  return lvm_fmt;
}

QStringList VolGroup::getLogVolNames()
{
  QStringList names;
  for(int x = 0; x < lv_count; x++){
    names << (member_lvs[x])->getName();
  }

  return names;
}

bool VolGroup::isWritable()
{
    return writable;
}

bool VolGroup::isResizable()
{
    return resizable;
}

bool VolGroup::isClustered()
{
    return clustered;
}

bool VolGroup::isPartial()
{
    return partial;
}

