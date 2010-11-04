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
    bool existing_pv;
    bool deleted_pv;
    m_mda_count = 0;
    vg_t lvm_vg;

    m_allocateable_extents = 0;
    m_pv_max       = 0;
    m_extent_size  = 0;
    m_extents      = 0;
    m_lv_max       = 0;
    m_size         = 0;
    m_free         = 0;
    m_free_extents = 0;
    m_exported     = false;
    m_partial      = false;
    m_clustered    = false;

    // clustered volumes can't be opened when clvmd isn't running 
    if( (lvm_vg = lvm_vg_open(lvm, m_vg_name.toAscii().data(), "r", NULL)) ){

        //    m_lvm_format   = QString("????"); // Set this from liblvm when available, see logvol->getLVMFormat()
        //    m_writable  = true; // ditto
        //    m_resizable = true; // ditto
        //    m_allocation_policy = // ditto again

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
		    if( QString( lvm_pv_get_uuid( pv_list->pv ) ).trimmed() == m_member_pvs[x]->getUuid() ){
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
		    if( QString( lvm_pv_get_uuid( pv_list->pv ) ).trimmed() == m_member_pvs[x]->getUuid() )
		      deleted_pv = false;
		}
		if(deleted_pv){
		    delete m_member_pvs.takeAt(x);
		}
	    }
        
	    for(int x = 0; x < m_member_pvs.size(); x++){
	        if( m_member_pvs[x]->isAllocateable() )
		    m_allocateable_extents += m_member_pvs[x]->getUnused() / (long long) m_extent_size;
		m_mda_count += m_member_pvs[x]->getMDACount();
	    }
	    
	    for(int x = m_member_lvs.size() - 1; x >= 0; x--) 
	        delete m_member_lvs.takeAt(x);
		
	}
	else
	    qDebug() << " Empty pv_dm_list?";
	
	lvm_vg_close(lvm_vg);
    }
    else{
        m_member_pvs.clear();
        m_member_lvs.clear();
    }

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
	if(name.trimmed() == m_member_pvs[x]->getDeviceName() && !name.contains("unknown device"))
	    return m_member_pvs[x];
    }

    return NULL;
}

void VolGroup::addLogicalVolume(LogVol *logicalVolume)
{
    LogVol *lv;
    PhysVol *pv;
    QList<long long> starting_extent;
    QStringList pv_name_list;
    QString pv_name, vg_attr;
    long long last_extent, last_used_extent;

    logicalVolume->setVolumeGroup(this);
    m_member_lvs.append(logicalVolume);
    m_lvm_format = logicalVolume->getLVMFormat();

    if( logicalVolume->isActive() ){
        pv_name_list = logicalVolume->getDevicePathAll();
        for(int x = 0; x < pv_name_list.size(); x++){
            if( (pv = getPhysVolByName(pv_name_list[x])) )
                pv->setActive();
        }
        pv_name_list.clear();
    }

    vg_attr = logicalVolume->getVGAttr();

    if(vg_attr.at(1) == 'z')
        m_resizable = true;
    else
        m_resizable = false;

    if(vg_attr.at(4) == 'c')
        m_allocation_policy = "contiguous";
    else if(vg_attr.at(4) == 'l')
        m_allocation_policy = "cling";
    else if(vg_attr.at(4) == 'n')
        m_allocation_policy = "normal";
    else if(vg_attr.at(4) == 'a')
        m_allocation_policy = "anywhere";

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

    // We are assuming mirrored logs only come in twos.

    LogVol *origin;
    if( logicalVolume->isMirrorLog() && !logicalVolume->isMirrorLeg() ){
        origin = getLogVolByName( logicalVolume->getOrigin() ); 
        if(origin){
            if( logicalVolume->isMirror() )
                origin->setLogCount(2);
            else
                origin->setLogCount(1);
        }
    }

    QStringList devices;
    for(int x = 0; x < m_member_lvs.size(); x++){
        for(int y = 0; y < m_member_lvs.size(); y++){

            devices = m_member_lvs[y]->getDevicePathAll();

            for(int z = 0; z < devices.size(); z++){
                if( m_member_lvs[x]->getName().remove("[").remove("]") == devices[z])
                    m_member_lvs[x]->setOrigin( m_member_lvs[y]->getName() );
            }
        }
    }
    return;
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

int VolGroup::getMDACount()
{
    return m_mda_count;
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
    return m_lvm_format;
}

QStringList VolGroup::getLogVolNames()
{
    QStringList names;
    for(int x = 0; x < m_member_lvs.size(); x++)
        names << (m_member_lvs[x])->getName();

    return names;
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

