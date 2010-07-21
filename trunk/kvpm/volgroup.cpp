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

#include "logvol.h"
#include "physvol.h"
#include "volgroup.h"

VolGroup::VolGroup(lvm_t lvm, const char *vgname)
{
    m_vg_name = QString(vgname).trimmed();
    rescan(lvm);
}

VolGroup::~VolGroup()
{
    for(int x = m_member_lvs.size() - 1; x >= 0; x--) 
        delete m_member_lvs.takeAt(x);
}

void VolGroup::rescan(lvm_t lvm)
{
    vg_t lvm_vg = lvm_vg_open(lvm, m_vg_name.toAscii().data(), "r", NULL);
    bool existing_pv;
    bool deleted_pv;

    m_lvm_fmt   = QString("????");       // Fix Me!!!
    m_writable  = true; // Fix me!!!
    m_resizable = true; // Fix me!!!
    m_allocation_policy    = "normal";  // Fix me!!!
    m_allocateable_extents = 0;

    m_pv_max       = lvm_vg_get_max_pv(lvm_vg); 
    m_extent_size  = lvm_vg_get_extent_size(lvm_vg);
    m_extents      = lvm_vg_get_extent_count(lvm_vg);
    m_lv_max       = lvm_vg_get_max_lv(lvm_vg);
    m_size         = lvm_vg_get_size(lvm_vg);
    m_free         = lvm_vg_get_free_size(lvm_vg);
    m_free_extents = lvm_vg_get_free_extent_count(lvm_vg);
    m_exported     = (bool)lvm_vg_is_exported(lvm_vg); 
    m_partial      = (bool)lvm_vg_is_partial(lvm_vg); 
    m_clustered    = (bool)lvm_vg_is_clustered(lvm_vg);

    dm_list* pv_dm_list = lvm_vg_list_pvs(lvm_vg);
    lvm_pv_list *pv_list;

    if(pv_dm_list){  // This should never be empty

        dm_list_iterate_items(pv_list, pv_dm_list){ // rescan() existing PhysVols 
            existing_pv = false;
            for(int x = 0; x < m_member_pvs.size(); x++){
                if( QString( lvm_pv_get_name( pv_list->pv ) ).trimmed() == m_member_pvs[x]->getDeviceName() ){
                    existing_pv = true;
                    m_member_pvs[x]->rescan( pv_list->pv );
                }
            }
            if( !existing_pv )
                m_member_pvs.append( new PhysVol( pv_list->pv, this ) );
        }
    
        for(int x = m_member_pvs.size() - 1; x >= 0; x--){ // delete PhysVolGroup if the pv is gone
            deleted_pv = true;
            dm_list_iterate_items(pv_list, pv_dm_list){ 
                if( QString( lvm_pv_get_name( pv_list->pv ) ).trimmed() == m_member_pvs[x]->getDeviceName() )
                    deleted_pv = false;
            }
            if(deleted_pv){
                delete m_member_pvs.takeAt(x);
            }
        }
        
        for(int x = 0; x < m_member_pvs.size(); x++)
            m_allocateable_extents += m_member_pvs.last()->getUnused() / (long long) m_extent_size;
        
        lvm_vg_close(lvm_vg);
        
        for(int x = m_member_lvs.size() - 1; x >= 0; x--) 
            delete m_member_lvs.takeAt(x);

    }
    else
        qDebug() << " Empty pv_dm_list?";

    return;
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
    for(int x = 0; x < m_member_lvs.size(); x++){
	if(name == m_member_lvs[x]->getName())
	    return m_member_lvs[x];
    }

    return NULL;
}

PhysVol* VolGroup::getPhysVolByName(QString name)
{
    for(int x = 0; x < m_member_pvs.size(); x++){
	if(name.trimmed() == m_member_pvs[x]->getDeviceName())
	    return m_member_pvs[x];
    }

    return NULL;
}

void VolGroup::addLogicalVolume(LogVol *logicalVolume)
{
    LogVol *lv;
    QList<long long> starting_extent;
    QStringList pv_name_list;
    QString pv_name;
    long long last_extent, last_used_extent;

    logicalVolume->setVolumeGroup(this);
    m_member_lvs.append(logicalVolume);

    for(int z = 0; z < m_member_pvs.size(); z++){
        last_extent = 0;
        last_used_extent = 0;
        pv_name = m_member_pvs[z]->getDeviceName();
        for(int x = 0; x < m_member_lvs.size() ; x++){
            lv = m_member_lvs[x];
            for(int segment = 0; segment < lv->getSegmentCount(); segment++){
                pv_name_list = lv->getDevicePath(segment);
                starting_extent = lv->getSegmentStartingExtent(segment);
                for(int y = 0; y < pv_name_list.size() ; y++){
                    if( pv_name == pv_name_list[y] ){
                        last_extent = starting_extent[y] - 1 + (lv->getSegmentExtents(segment) / (lv->getSegmentStripes(segment)));
                        if( last_extent > last_used_extent )
                            last_used_extent = last_extent;
                    }
                }
            }
        }
        m_member_pvs[z]->setLastUsedExtent(last_used_extent);
    }
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
    return m_member_lvs.size();
}

int VolGroup::getLogVolMax()
{
    return m_lv_max;
}

int VolGroup::getPhysVolCount()
{
    return m_member_pvs.size();
}

int VolGroup::getPhysVolMax()
{
    return m_pv_max;
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
    for(int x = 0; x < m_member_lvs.size(); x++)
        names << (m_member_lvs[x])->getName();

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

bool VolGroup::isExported()
{
    return m_exported;
}

