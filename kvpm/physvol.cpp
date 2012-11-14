/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "physvol.h"

#include "logvol.h"
#include "volgroup.h"

#include <QByteArray>
#include <QDebug>
#include <QtAlgorithms>
#include <QListIterator>



namespace {
    bool isLessThan(const LVSegmentExtent * lv1 , const LVSegmentExtent * lv2)
    {
        return lv1->first_extent < lv2->first_extent;
    }
}


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
    if (value.is_valid)
        flags.append(value.value.string, 3);

    value = lvm_pv_get_property(lvm_pv, "pv_tags");
    m_tags.clear();
    if (value.is_valid)
        m_tags = QString(value.value.string).split(',');
    for (int x = 0; x < m_tags.size(); x++)
        m_tags[x] = m_tags[x].trimmed();

    value = lvm_pv_get_property(lvm_pv, "pv_mda_used_count");
    if (value.is_valid)
        m_mda_used = value.value.integer;

    if (flags[0] == 'a')
        m_allocatable = true;
    else
        m_allocatable = false;

    if (flags[2] == 'm')
        m_missing = true;
    else
        m_missing = false;

    m_last_used_extent = 0;
    m_active        = false;     // pv is active if any associated lvs are active
    m_device        = QString(lvm_pv_get_name(lvm_pv));
    m_device_size   = lvm_pv_get_dev_size(lvm_pv);
    m_unused        = lvm_pv_get_free(lvm_pv);
    m_size          = lvm_pv_get_size(lvm_pv);
    m_uuid          = QString(lvm_pv_get_uuid(lvm_pv));
    m_mda_count     = lvm_pv_get_mda_count(lvm_pv);

    value = lvm_pv_get_property(lvm_pv, "pv_mda_size");
    if (value.is_valid)
        m_mda_size = value.value.integer;

    /*
    // The following wil be used to to calculate the last used
    // segement once the "lv_name" property gets implemented

    // Finding segments with lvs locked against change (snaps mirrors and pvmoves)
    // will also be possible

    // remove clunky code from pvtree.cpp when this get implemented!

    dm_list* pvseg_dm_list = lvm_pv_list_pvsegs(lvm_pv);
    lvm_pvseg_list *pvseg_list;

    if(pvseg_dm_list){
        dm_list_iterate_items(pvseg_list, pvseg_dm_list){

            value = lvm_pvseg_get_property( pvseg_list->pvseg , "lv_name");

            qDebug() << "Name: " << value.value.string;

            value = lvm_pvseg_get_property( pvseg_list->pvseg , "lv_attr");

            qDebug() << "attr: " << value.value.string;

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

VolGroup* PhysVol::getVg()
{
    return m_vg;
}

QString PhysVol::getName()
{
    return m_device.trimmed();
}

QString PhysVol::getUuid()
{
    return m_uuid.trimmed();
}

QStringList PhysVol::getTags()
{
    return m_tags;
}

bool PhysVol::isAllocatable()
{
    return m_allocatable;
}

bool PhysVol::isMissing()
{
    return m_missing;
}

bool PhysVol::isActive()
{
    return m_active;
}

void PhysVol::setActive()
{
    m_active = true;
}

long PhysVol::getMdaCount()
{
    return m_mda_count;
}

long PhysVol::getMdaUsed()
{
    return m_mda_used;
}

long long PhysVol::getMdaSize()
{
    return m_mda_size;   // size in bytes
}

long long PhysVol::getSize()
{
    return m_size;
}

long long PhysVol::getDeviceSize()
{
    return m_device_size;
}

long long PhysVol::getRemaining()
{
    return m_unused;
}

int PhysVol::getPercentUsed()
{
    int percent;

    if (m_unused == 0)
        return 100;
    else if (m_unused == m_size)
        return 0;
    else if (m_size == 0)       // This shouldn't happen
        return 100;
    else
        percent = qRound(((m_size - m_unused) * 100.0) / m_size);

    return percent;
}

long long PhysVol::getLastUsedExtent()
{
    return m_last_used_extent;
}

void PhysVol::setLastUsedExtent(const long long last)
{
    m_last_used_extent = last;
}


// Returns a list of all the lv segments on the pv sorted by the
// extent. Ordered from extent first to last extent.
// Must not be called until after the LogVols have been scanned

QList<LVSegmentExtent *> PhysVol::sortByExtent()
{
    QList<long long> first_extent_list;
    QStringList pv_name_list;
    QList<LVSegmentExtent *> lv_extents;
    LVSegmentExtent *temp;
    LogVol *lv;

    QListIterator<LogVol *> lv_itr(getVg()->getLogicalVolumesFlat());

    while (lv_itr.hasNext()) {
        lv = lv_itr.next();
        if (!lv->isSnapContainer()) {
            for (int segment = lv->getSegmentCount() - 1; segment >= 0; segment--) {
                pv_name_list = lv->getPvNames(segment);
                first_extent_list = lv->getSegmentStartingExtent(segment);
                for (int y = pv_name_list.size() - 1; y >= 0; y--) {
                    if (pv_name_list[y] == getName()) {
                        temp = new LVSegmentExtent;
                        temp->lv_name = lv->getName();
                        temp->first_extent = first_extent_list[y];
                        temp->last_extent = temp->first_extent - 1 + (lv->getSegmentExtents(segment) / (lv->getSegmentStripes(segment)));
                        lv_extents.append(temp);
                    }
                }
            }
        }
    }

    qSort(lv_extents.begin() , lv_extents.end(), isLessThan);

    return lv_extents;
}

long long PhysVol::getContiguous(LogVol *const lv)
{
    long long contiguous = 0;
    const long long extent_size = m_vg->getExtentSize();
    const long long end = (m_size / extent_size) - 1;
    const QList<LVSegmentExtent *> lv_extents = sortByExtent();

    QList<LogVol *> legs;

    //
    // TEST UNDER CONVERSION !!!
    //
    
    if (lv == NULL)
        return getContiguous();
    
    if (lv->isMirror() || lv->isRaid()) {
        legs.append(lv->getAllChildrenFlat());
        
        for (int n = legs.size() - 1; n >= 0; n--) {
            if ( !(legs[n]->isRaidImage() || (legs[n]->isLvmMirrorLeg() && !legs[n]->isLvmMirrorLog())) ) 
                legs.removeAt(n);
        }
    } else {
        legs.append(lv);
    }

    for (int x = 0; x < legs.size(); x++) {
        
        const int last_segment = legs[x]->getSegmentCount() - 1;
        const QStringList pv_names = legs[x]->getPvNames(last_segment);
        long long last_extent;
        
        for (int y = 0; y < pv_names.size(); y++) {
            if (pv_names[y] == m_device) {
                
                last_extent = legs[x]->getSegmentStartingExtent(last_segment)[y];
                last_extent += (legs[x]->getSegmentExtents(last_segment) / pv_names.size()) - 1;
                
                for (int z = 0; z < lv_extents.size(); z++) {
                    if (lv_extents[z]->first_extent > last_extent) {
                        contiguous = (lv_extents[z]->first_extent - last_extent) - 1;
                        break;
                    } else if (z == lv_extents.size() - 1) {
                        contiguous = end - last_extent;
                    }
                }
            } 
        }
    }

    for (int x = 0; x < lv_extents.size(); x++)
        delete lv_extents[x];
    
    return contiguous * extent_size;
}

long long PhysVol::getContiguous()
{
    long long contiguous = 0;
    const long long extent_size = m_vg->getExtentSize();
    const long long end = (m_size / extent_size) - 1;
    const QList<LVSegmentExtent *> lv_extents = sortByExtent();

    if (lv_extents.size() > 0) {

        for (int x = lv_extents.size() - 1; x >= 0 ; x--) {
            if (x == (lv_extents.size() - 1)) {
                if (end - lv_extents[x]->last_extent > contiguous)
                    contiguous = end - lv_extents[x]->last_extent;
            } else {
                if ((lv_extents[x + 1]->first_extent - lv_extents[x]->last_extent) - 1 > contiguous)
                    contiguous = (lv_extents[x + 1]->first_extent - lv_extents[x]->last_extent) - 1;
            }
        }
        
        if (lv_extents[0]->first_extent > contiguous)
            contiguous = lv_extents[0]->first_extent;
        
    } else {   // empty pv
        contiguous = end + 1;
    }
    
    for (int x = 0; x < lv_extents.size(); x++)
        delete lv_extents[x];

    return contiguous * extent_size;
}
