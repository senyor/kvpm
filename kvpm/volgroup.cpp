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


#include "volgroup.h"

#include <QDebug>
#include <QWidget>

#include "mounttables.h"
#include "physvol.h"


VolGroup::VolGroup(lvm_t lvm, const char *vgname, MountTables *const tables)
{
    m_vg_name = QString(vgname).trimmed();
    m_tables = tables;
    rescan(lvm);
}

VolGroup::~VolGroup()
{
    for (auto ptr : m_member_pvs)
        delete ptr;
}

void VolGroup::rescan(lvm_t lvm)
{
    vg_t lvm_vg;
    lvm_property_value value;

    m_allocatable_extents = 0;
    m_mda_count    = 0;
    m_mda_used     = 0;
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
    m_open_failed  = false;

    // clustered volumes can't be opened when clvmd isn't running

    QByteArray  vg_name_qba = m_vg_name.toLocal8Bit();
    const char *vg_name_ascii = vg_name_qba.data();

    if ((lvm_vg = lvm_vg_open(lvm, vg_name_ascii, "r", 0x00))) {

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
        m_uuid         = QString(lvm_vg_get_uuid(lvm_vg));

        value = lvm_vg_get_property(lvm_vg, "vg_fmt");
        m_lvm_format = QString(value.value.string);

        value = lvm_vg_get_property(lvm_vg, "vg_attr");
        const QString vg_attr = QString(value.value.string);

        if (vg_attr.at(0) == 'w')
            m_writable = true;
        else
            m_writable = false;

        if (vg_attr.at(1) == 'z')
            m_resizable = true;
        else
            m_resizable = false;

        if (vg_attr.at(4) == 'c')
            m_policy = CONTIGUOUS;
        else if (vg_attr.at(4) == 'l')
            m_policy = CLING;
        else if (vg_attr.at(4) == 'a')
            m_policy = ANYWHERE;
        else
            m_policy = NORMAL;

        processPhysicalVolumes(lvm_vg);
        processLogicalVolumes(lvm_vg);

        lvm_vg_close(lvm_vg);
    } else {
        if (QString(lvm_errmsg(lvm)).contains("cluster"))
            m_clustered = true;

        m_open_failed = true;

        for (int x = m_member_pvs.size() - 1; x >= 0; x--)
            delete m_member_pvs.takeAt(x);

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

lv_t VolGroup::findOrphan(QList<lv_t> &childList)
{
    LvList all_lvs = getLogicalVolumesFlat();
    QList<lv_t> orphan_list;
    lvm_property_value value;
    QString child_name;
    QString lv_attr;
    lv_t orphan = nullptr;

    for (int x = all_lvs.size() - 1; x >= 0; x--) {
        for (int y = childList.size() - 1; y >= 0; y--) {
            value = lvm_lv_get_property(childList[y], "lv_name");
            child_name = QString(value.value.string).trimmed();
            if (child_name == all_lvs[x]->getName()) {
                childList.removeAt(y);
                all_lvs.removeAt(x);
                break;
            }
        }
    }

    for (const auto &child : childList) { // sort mirrors first, loners last

        value = lvm_lv_get_property(child, "lv_attr");
        lv_attr = QString(value.value.string);

        if (lv_attr[0] == 'm' || lv_attr[0] == 'M')
            orphan_list.prepend(child);
        else
            orphan_list.append(child);

    }

    if (!orphan_list.isEmpty()) {
        orphan = orphan_list.takeAt(0);
        childList = orphan_list;
        return orphan;
    }

    childList = orphan_list;
    return nullptr;
}

LvList VolGroup::getLogicalVolumes() const  // *TOP LEVEL ONLY* snapcontainers returned not snaps and origin
{ 
    LvList members;
    for (auto lv : m_member_lvs)
        members << lv.data();

    return members; 
}


LvList VolGroup::getLogicalVolumesFlat() const
{
    LvList flat_list;

    for (auto lv : m_member_lvs) {
        flat_list << lv.data();
        flat_list << lv->getAllChildrenFlat();
    }

    return flat_list;
}

LogVol * VolGroup::getLvByName(QString shortName) const // Do not return snap container, just the "real" lv
{
    QListIterator<LogVol *> lvs_itr(getLogicalVolumesFlat());
    shortName = shortName.trimmed();

    while (lvs_itr.hasNext()) {
        LogVol *lv = lvs_itr.next();
        if (shortName == lv->getName() && !lv->isSnapContainer())
            return lv;
    }

    return nullptr;
}

LogVol * VolGroup::getLvByUuid(QString uuid) const  // Do not return snap container, just the "real" lv
{
    QListIterator<LogVol *> lvs_itr(getLogicalVolumesFlat());
    uuid = uuid.trimmed();

    while (lvs_itr.hasNext()) {
        LogVol *lv = lvs_itr.next();
        if (uuid == lv->getUuid() && !lv->isSnapContainer())
            return lv;
    }

    return nullptr;
}

PhysVol* VolGroup::getPvByName(QString name) const
{
    QListIterator<PhysVol *> pvs_itr(m_member_pvs);
    name = name.trimmed();

    if (!name.contains("unknown device")) {
        while (pvs_itr.hasNext()) {
            PhysVol *pv = pvs_itr.next();
            if (name == pv->getMapperName())
                return pv;
        }
    }

    return nullptr;
}

QStringList VolGroup::getLvNames() const 
{
    QStringList names;

    for (const auto lv : m_member_lvs)
        names << lv->getName();

    return names;
}

void VolGroup::processPhysicalVolumes(vg_t lvmVG)
{
    dm_list* pv_dm_list = lvm_vg_list_pvs(lvmVG);
    lvm_pv_list *pv_list;

    if (pv_dm_list) { // This should never be empty

        dm_list_iterate_items(pv_list, pv_dm_list) { // rescan() existing PhysVols
            bool existing_pv = false;
            for (int x = 0; x < m_member_pvs.size(); x++) {
                if (QString(lvm_pv_get_uuid(pv_list->pv)).trimmed() == m_member_pvs[x]->getUuid()) {
                    existing_pv = true;
                    m_member_pvs[x]->rescan(pv_list->pv);
                }
            }
            if (!existing_pv)
                m_member_pvs.append(new PhysVol(pv_list->pv, this));
        }

        for (int x = m_member_pvs.size() - 1; x >= 0; x--) { // delete PhysVolGroup if the pv is gone
            bool deleted_pv = true;
            dm_list_iterate_items(pv_list, pv_dm_list) {
                if (QString(lvm_pv_get_uuid(pv_list->pv)).trimmed() == m_member_pvs[x]->getUuid())
                    deleted_pv = false;
            }
            if (deleted_pv)
                delete m_member_pvs.takeAt(x);
        }

        for (int x = 0; x < m_member_pvs.size(); x++) {
            if (m_member_pvs[x]->isAllocatable())
                m_allocatable_extents += m_member_pvs[x]->getRemaining() / m_extent_size;
            m_mda_count += m_member_pvs[x]->getMdaCount();
            m_mda_used += m_member_pvs[x]->getMdaUsed();
        }

    } else {
        qDebug() << " Empty pv_dm_list?";
    }
}

void VolGroup::processLogicalVolumes(vg_t lvmVG)
{
    lvm_property_value value;
    dm_list* lv_dm_list = lvm_vg_list_lvs(lvmVG);
    lvm_lv_list *lv_list;
    QList<lv_t> lvm_lvs_all_top;       // top level lvm logical volume handles
    QList<lv_t> lvm_lvs_all_children;  // children of top level volumes
    QByteArray flags;

    QList<SmrtLvPtr> old_member_lvs = m_member_lvs;
    m_member_lvs.clear();
    m_lv_names_all.clear();

    if (lv_dm_list) {

        dm_list_iterate_items(lv_list, lv_dm_list) {

            value = lvm_lv_get_property(lv_list->lv, "lv_name");
            if (QString(value.value.string).trimmed().startsWith("snapshot"))
                continue;

            value = lvm_lv_get_property(lv_list->lv, "lv_name");
            const QString top_name = QString(value.value.string).trimmed();
            m_lv_names_all << top_name;
            
            if (top_name.contains("_mlog")  || top_name.contains("_mimage") || 
                top_name.contains("_tmeta") || top_name.contains("_rmeta")  || 
                top_name.contains("_tdata") || top_name.contains("_rimage")) {

                lvm_lvs_all_children.append(lv_list->lv);
            } else {
                value = lvm_lv_get_property(lv_list->lv, "lv_attr");
                flags = value.value.string;

                switch (flags[0]) {
                case '-':
                case 'p':
                case 't':
                    lvm_lvs_all_top.append(lv_list->lv);
                    break;
                case 'c':
                case 'M':
                case 'm':
                case 'r':
                case 'O':
                case 'o':
                    if (flags[6] == 't')
                        lvm_lvs_all_children.append(lv_list->lv);
                    else
                        lvm_lvs_all_top.append(lv_list->lv);
                    break;
                case 'V':
                case 'e':
                default:
                    lvm_lvs_all_children.append(lv_list->lv);
                    break;
                }
            }
        }

        for (auto lvm_lv : lvm_lvs_all_top) { // rescan() existing LogVols
            bool is_new = true;

            for (int x = old_member_lvs.size() - 1; x >= 0; x--) {

                value = lvm_lv_get_property(lvm_lv, "lv_attr");
                flags = value.value.string;

                if (QString(lvm_lv_get_name(lvm_lv)).trimmed() == old_member_lvs[x]->getName()) {
                    old_member_lvs[x]->rescan(lvm_lv, lvmVG);
                    m_member_lvs.append(old_member_lvs.takeAt(x));
                    is_new = false;
                }
            }

            if (is_new) {
                m_member_lvs << SmrtLvPtr(new LogVol(lvm_lv, lvmVG, this, nullptr, m_tables));
            }
        }

        old_member_lvs.clear();

        lv_t lvm_lv_orphan;                // non-top lvm logical volume handle with no home
        while ((lvm_lv_orphan = findOrphan(lvm_lvs_all_children))) 
            m_member_lvs << SmrtLvPtr(new LogVol(lvm_lv_orphan, lvmVG, this, nullptr, m_tables, true));

    } else {   // lv_dm_list is empty so clean up member lvs
        m_member_lvs.clear();
    }
}

void VolGroup::setActivePhysicalVolumes()
{
    PhysVol *pv;
    const LvList all_lvs = getLogicalVolumesFlat();

    for (int x = all_lvs.size() - 1; x >= 0; x--) {
        if (all_lvs[x]->isActive()) {
            m_active = true;
            const QStringList pv_name_list = all_lvs[x]->getPvNamesAllFlat();

            for (int x = pv_name_list.size() - 1; x >= 0; x--) {
                if ((pv = getPvByName(pv_name_list[x])))
                    pv->setActive();
            }
        }
    }
}

// TODO -- this should use pvseg properties pvseg_start pvseg_size
// It should be moved to PhysVol
void VolGroup::setLastUsedExtent()
{
    for (auto pv : m_member_pvs) {
        long long last_extent = 0;
        long long last_used_extent = 0;
        const QString pv_name = pv->getMapperName();

        for (auto smart : m_member_lvs) {
            auto lv = smart.data();

            for (int segment = lv->getSegmentCount() - 1; segment >= 0; segment--) {
                QList<long long> starting_extent = lv->getSegmentStartingExtent(segment);
                const QStringList pv_name_list = lv->getPvNames(segment);

                for (int i = 0; i < pv_name_list.size(); i++) {
                    if (pv_name == pv_name_list[i]) {
                        last_extent = starting_extent[i] - 1 + (lv->getSegmentExtents(segment) / (lv->getSegmentStripes(segment)));
                        if (last_extent > last_used_extent)
                            last_used_extent = last_extent;
                    }
                }
            }
        }

        pv->setLastUsedExtent(last_used_extent);
    }
}
