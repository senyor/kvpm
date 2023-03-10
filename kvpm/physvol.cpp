/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013, 2014 Benjamin Scott   <benscott@nwlink.com>
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
#include "misc.h"
#include "volgroup.h"

#include <QByteArray>
#include <QDebug>
#include <QtAlgorithms>



namespace {
    bool isLessThan(const LVSegmentExtent * lv1 , const LVSegmentExtent * lv2)
    {
        return lv1->first_extent < lv2->first_extent;
    }
}


PhysVol::PhysVol(pv_t pv, const VolGroup *const vg) 
    : m_vg(vg)
{
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
    m_active      = false;     // pv is active if any associated lvs are active
    m_device      = QString(lvm_pv_get_name(lvm_pv));
    m_device_size = lvm_pv_get_dev_size(lvm_pv);
    m_unused      = lvm_pv_get_free(lvm_pv);
    m_size        = lvm_pv_get_size(lvm_pv);
    m_uuid        = QString(lvm_pv_get_uuid(lvm_pv));
    m_mda_count   = lvm_pv_get_mda_count(lvm_pv);

    value = lvm_pv_get_property(lvm_pv, "pv_mda_size");
    if (value.is_valid)
        m_mda_size = value.value.integer;

    m_mapper_device = findMapperPath(m_device);

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

int PhysVol::getPercentUsed() const
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

// Returns a list of all the lv segments on the pv sorted by the
// extent. Ordered from extent first to last extent.
// Must not be called until after the LogVols have been scanned

QList<LVSegmentExtent *> PhysVol::sortByExtent() const
{
    QList<LVSegmentExtent *> lv_extents;

    for (auto lv : getVg()->getLogicalVolumesFlat()) {
        if (!lv->isSnapContainer()) {
            for (int seg = lv->getSegmentCount() - 1; seg >= 0; --seg) {
                QStringList pv_name_list = lv->getPvNames(seg);
                QList<long long> first_extent_list = lv->getSegmentStartingExtent(seg);
                for (int i = pv_name_list.size() - 1; i >= 0; --i) {
                    if (pv_name_list[i] == getMapperName()) {
                        LVSegmentExtent *const temp = new LVSegmentExtent;
                        temp->lv_name = lv->getName();
                        temp->first_extent = first_extent_list[i];
                        temp->last_extent = temp->first_extent - 1 + (lv->getSegmentExtents(seg) / (lv->getSegmentStripes(seg)));
                        lv_extents.append(temp);
                    }
                }
            }
        }
    }

    qSort(lv_extents.begin() , lv_extents.end(), isLessThan);

    return lv_extents;
}

/*  Find the contiguous additional free space on this pv for
    the specified lv. For thin pools, RAID and mirrors
    it drills down to the lower levels. Returns zero if
    the lv isn't using this pv already or it is only used
    for an element that normally isn't extensible such as a 
    log or metadata */

long long PhysVol::getContiguous(LogVol *lv) const
{
    long long contiguous = 0;
    const QList<LVSegmentExtent *> lv_extents = sortByExtent();

    LvList legs;
    
    if (lv == nullptr)
        return getContiguous();

    if (lv->isThinPool()) {
        for (auto data : lv->getAllChildrenFlat()) {
            if (data->isThinPoolData()) {
                lv = data;
                break;
            }
        }
    }
    
    if (lv->isMirror() || lv->isRaid()) {
        for (auto image : lv->getAllChildrenFlat()) {
            if (image->isRaidImage() || (image->isLvmMirrorLeg() && !image->isLvmMirrorLog())) 
                legs.append(image);
        }
    } else {
        legs.append(lv);
    }

    const long long extent_size = m_vg->getExtentSize();
    const long long end = (m_size / extent_size) - 1;

    for (auto leg : legs) {
        const int last_segment = leg->getSegmentCount() - 1;
        const QStringList pv_names = leg->getPvNames(last_segment);
        long long last_extent;
        
        for (int i = 0; i < pv_names.size(); ++i) {
            if (pv_names[i] == m_device) {
                
                last_extent = leg->getSegmentStartingExtent(last_segment)[i];
                last_extent += (leg->getSegmentExtents(last_segment) / pv_names.size()) - 1;
                
                for (int j = 0; j < lv_extents.size(); ++j) {
                    if (lv_extents[j]->first_extent > last_extent) {
                        contiguous = (lv_extents[j]->first_extent - last_extent) - 1;
                        break;
                    } else if (j == lv_extents.size() - 1) {
                        contiguous = end - last_extent;
                    }
                }
            } 
        }
    }

    for (auto ext : lv_extents)
        delete ext;
    
    return contiguous * extent_size;
}

long long PhysVol::getContiguous() const
{
    long long contiguous = 0;
    const long long extent_size = m_vg->getExtentSize();
    const long long end = (m_size / extent_size) - 1;
    const QList<LVSegmentExtent *> lv_extents = sortByExtent();

    if (lv_extents.size() > 0) {
        for (int i = lv_extents.size() - 1; i >= 0 ; i--) {
            if (i == (lv_extents.size() - 1)) {
                if (end - lv_extents[i]->last_extent > contiguous)
                    contiguous = end - lv_extents[i]->last_extent;
            } else {
                if ((lv_extents[i + 1]->first_extent - lv_extents[i]->last_extent) - 1 > contiguous)
                    contiguous = (lv_extents[i + 1]->first_extent - lv_extents[i]->last_extent) - 1;
            }
        }
        
        if (lv_extents[0]->first_extent > contiguous)
            contiguous = lv_extents[0]->first_extent;
    } else {   // empty pv
        contiguous = end + 1;
    }
    
    for (auto ext : lv_extents)
        delete ext;

    return contiguous * extent_size;
}

