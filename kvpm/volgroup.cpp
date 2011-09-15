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

#include "volgroup.h"

#include <QtGui>

#include "logvol.h"
#include "physvol.h"

VolGroup::VolGroup(lvm_t lvm, const char *vgname)
{
    m_vg_name = QString(vgname).trimmed();
    rescan(lvm);
}

VolGroup::~VolGroup()
{
    for(int x = m_member_lvs.size() - 1; x >= 0; x--) 
        delete m_member_lvs.takeAt(x);

    for(int x = m_member_pvs.size() - 1; x >= 0; x--) 
        delete m_member_pvs.takeAt(x);
}

void VolGroup::rescan(lvm_t lvm)
{
    vg_t lvm_vg;
    lvm_property_value value;
    QString vg_attr;
    QList<LogVol *> lv_children;

    m_allocatable_extents = 0;
    m_mda_count    = 0;
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
    m_active       = false;

    // clustered volumes can't be opened when clvmd isn't running 

    QByteArray  vg_name_array = m_vg_name.toAscii();
    const char *vg_name_ascii = vg_name_array.data();

    if( (lvm_vg = lvm_vg_open(lvm, vg_name_ascii, "r", 0x00 )) ){

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
        m_uuid         = QString( lvm_vg_get_uuid(lvm_vg) );

        value = lvm_vg_get_property(lvm_vg, "vg_fmt");
        m_lvm_format = QString(value.value.string);

        value = lvm_vg_get_property(lvm_vg, "vg_attr");
        vg_attr = QString(value.value.string);

        if(vg_attr.at(0) == 'w')
            m_writable = true;
        else
            m_writable = false;

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
        
        processPhysicalVolumes(lvm_vg);
        processLogicalVolumes(lvm_vg);

        lvm_vg_close(lvm_vg);
    }
    else{
        for(int x = m_member_pvs.size() - 1; x >= 0; x--) 
	    delete m_member_pvs.takeAt(x);
		
        for(int x = m_member_lvs.size() - 1; x >= 0; x--) 
	    delete m_member_lvs.takeAt(x);
		
        m_member_pvs.clear();
        m_member_lvs.clear();
    }
    
    setActivePhysicalVolumes();
    setLastUsedExtent();

    return;
}


/* Takes orphan volumes off the child lv list and puts them back onto
   the top level lv list. An orphan is a volume that should be hidden
   under another, like a mirror leg, but doesn't seem to have a parent,
   or something left behind by a misbehaving lvm process  */
 
QList<lv_t> VolGroup::findOrphans(QList<lv_t> childList)
{
    QList<LogVol *> all_lvs;
    lvm_property_value value;
    QString child_name;

    all_lvs = getLogicalVolumesFlat();

    for(int x = all_lvs.size() - 1; x >= 0; x--){
        for(int y = childList.size() - 1; y >= 0; y--){

            value = lvm_lv_get_property(childList[y], "lv_name");
            child_name = QString(value.value.string).trimmed();

            if( child_name == all_lvs[x]->getName() ){
                childList.removeAt(y);
                all_lvs.removeAt(x);
                break;
            }
        }
    }

    return childList;
}

const QList<LogVol *>  VolGroup::getLogicalVolumes()
{
    return m_member_lvs;
}

const QList<LogVol *>  VolGroup::getLogicalVolumesFlat()
{
    QList<LogVol *> tree_list = m_member_lvs;
    QList<LogVol *> flat_list;

    long tree_list_size = tree_list.size(); 

    for(int x = 0; x < tree_list_size; x++){
        flat_list.append(tree_list[x]);
        flat_list.append(tree_list[x]->getAllChildrenFlat());
    } 

    return flat_list;
}

const QList<PhysVol *> VolGroup::getPhysicalVolumes()
{
    return m_member_pvs;
}

LogVol* VolGroup::getLogVolByName(QString shortName)  // Do not return snap container, just the "real" lv
{
    QList<LogVol *> all_lvs = getLogicalVolumesFlat();
    const int lv_count = all_lvs.size();
    const QString name = shortName.trimmed();

    for(int x = 0; x < lv_count; x++){
	if( name == all_lvs[x]->getName() && !all_lvs[x]->isSnapContainer() )
	    return all_lvs[x];
    }

    return NULL;
}

LogVol* VolGroup::getLogVolByUuid(QString uuid)
{
    QList<LogVol *> all_lvs = getLogicalVolumesFlat();
    const int lv_count = all_lvs.size();
    uuid = uuid.trimmed();

    for(int x = 0; x < lv_count; x++){
	if(uuid == all_lvs[x]->getUuid() && !all_lvs[x]->isSnapContainer() )
	    return all_lvs[x];
    }

    return NULL;
}

PhysVol* VolGroup::getPhysVolByName(QString name)
{
    for(int x = 0; x < m_member_pvs.size(); x++){
	if(name.trimmed() == m_member_pvs[x]->getName() && !name.contains("unknown device"))
	    return m_member_pvs[x];
    }

    return NULL;
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

long long VolGroup::getAllocatableExtents()
{
    return m_allocatable_extents;
}

long long VolGroup::getAllocatableSpace()
{
    return m_allocatable_extents * (long long)m_extent_size;
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

QString VolGroup::getUuid()
{
    return m_uuid;
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

    for(int x = m_member_lvs.size() - 1; x >= 0; x--)
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

bool VolGroup::isActive()
{
    return m_active;
}

void VolGroup::processPhysicalVolumes(vg_t lvmVG)
{
    dm_list* pv_dm_list = lvm_vg_list_pvs(lvmVG);
    lvm_pv_list *pv_list;
    bool existing_pv, deleted_pv;
	
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
            if( m_member_pvs[x]->isAllocatable() )
                m_allocatable_extents += m_member_pvs[x]->getUnused() / (long long) m_extent_size;
            m_mda_count += m_member_pvs[x]->getMDACount();
        }
	
    }
    else
        qDebug() << " Empty pv_dm_list?";
}

void VolGroup::processLogicalVolumes(vg_t lvmVG)
{
    lvm_property_value value;
    bool is_new;
    dm_list* lv_dm_list = lvm_vg_list_lvs(lvmVG);
    lvm_lv_list *lv_list;
    QString lv_name;
    QString top_name;
    QList<lv_t> lvm_lvs_all_top;       // top level lvm logical volume handles
    QList<lv_t> lvm_lvs_all_children;  // top level lvm logical volume handles
    QList<lv_t> lvm_lvs_orphans;       // non-top lvm logical volume handles with no home
    QByteArray flags;
    
    QList<LogVol *> old_member_lvs = m_member_lvs;
    m_member_lvs.clear();

    if(lv_dm_list){
	
        dm_list_iterate_items(lv_list, lv_dm_list){ 
            
            value = lvm_lv_get_property(lv_list->lv, "lv_name");
            if( QString(value.value.string).trimmed().startsWith("snapshot") )
                continue;
            
            value = lvm_lv_get_property(lv_list->lv, "lv_attr");
            flags = value.value.string;
            
            switch( flags[0] ){
            case '-':
            case 'c':
            case 'O':
            case 'o':
            case 'p':
                lvm_lvs_all_top.append( lv_list->lv );
                break;
            case 'M': 
            case 'm':
                value = lvm_lv_get_property(lv_list->lv, "lv_name");
                top_name = QString(value.value.string).trimmed();
                if( !(top_name.endsWith("_mlog") || top_name.contains("_mimagetmp_")) )
                    lvm_lvs_all_top.append( lv_list->lv );
                break;
            default:
                lvm_lvs_all_children.append( lv_list->lv );
                break;
            }
        }
        
        for(int y = 0; y < lvm_lvs_all_top.size(); y++ ){ // rescan() existing LogVols 
            is_new = true;

            value = lvm_lv_get_property(lvm_lvs_all_top[y], "lv_name");
            lv_name = QString(value.value.string).trimmed();

            for(int x = old_member_lvs.size() - 1; x >= 0; x--){

                value = lvm_lv_get_property(lvm_lvs_all_top[y], "lv_attr");
                flags = value.value.string;

                if( QString(lvm_lv_get_name( lvm_lvs_all_top[y] )).trimmed() == old_member_lvs[x]->getName() ){
                    old_member_lvs[x]->rescan(lvm_lvs_all_top[y], lvmVG);
                    m_member_lvs.append( old_member_lvs.takeAt(x) );
                    is_new = false;
                }
            }

            if(is_new){
                m_member_lvs.append( new LogVol( lvm_lvs_all_top[y], lvmVG, this, NULL ) );
            }
        }

        for(int x = old_member_lvs.size() - 1; x >= 0; x--){   // delete LogVol if lv is gone, always delete orphans
            delete old_member_lvs.takeAt(x);
        }

        lvm_lvs_orphans = findOrphans(lvm_lvs_all_children);

        for(int x = lvm_lvs_orphans.size() - 1; x >= 0; x--)
            m_member_lvs.append( new LogVol( lvm_lvs_orphans[x], lvmVG, this, NULL, true ) );
    }
    else{      // lv_dm_list is empty so clean up member lvs 
        for(int x = m_member_lvs.size() - 1; x >= 0; x--) 
            delete m_member_lvs.takeAt(x);
    }
}

void VolGroup::setActivePhysicalVolumes()
{
    QStringList pv_name_list;
    PhysVol *pv;

    for(int x = 0; x < m_member_lvs.size(); x++){
        if( m_member_lvs[x]->isActive() ){
            m_active = true;
	    pv_name_list = m_member_lvs[x]->getDevicePathAll();
	    for(int x = 0; x < pv_name_list.size(); x++){
	        if( (pv = getPhysVolByName(pv_name_list[x])) )
		    pv->setActive();
	    }
	    pv_name_list.clear();
	}
    }
}

// TODO -- this should use pvseg properties pvseg_start pvseg_size
// It should be moved to PhysVol
void VolGroup::setLastUsedExtent()
{
    long long last_extent, last_used_extent;
    LogVol *lv;
    QString pv_name;
    QList<long long> starting_extent;
    QStringList pv_name_list;

    for(int z = m_member_pvs.size() - 1; z >= 0; z--){
        last_extent = 0;
        last_used_extent = 0;
        pv_name = m_member_pvs[z]->getName();
        for(int x = m_member_lvs.size() - 1; x >= 0; x--){
            lv = m_member_lvs[x];
            for(int segment = lv->getSegmentCount() - 1; segment >= 0; segment--){
                pv_name_list = lv->getDevicePath(segment);
                starting_extent = lv->getSegmentStartingExtent(segment);
                for(int y = pv_name_list.size() - 1; y >= 0; y--){
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
