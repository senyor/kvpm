/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "pvtree.h"

#include <KGlobal>
#include <KConfigSkeleton>
#include <KIcon>
#include <KLocale>

#include <QDebug>
#include <QPoint>
#include <QMenu>


#include "logvol.h"
#include "masterlist.h"
#include "processprogress.h"
#include "pvmove.h"
#include "pvchange.h"
#include "physvol.h"
#include "topwindow.h"
#include "volgroup.h"


/* This is the physical volume tree list on the volume group tab */

PVTree::PVTree(VolGroup *const group, QWidget *parent) : 
    QTreeWidget(parent),
    m_vg(group)
{
    QStringList header_labels;
    setColumnCount(6);
    QTreeWidgetItem *item;

    header_labels << i18nc("The name of the device", "Name") << i18n("Size")
                  << i18nc("Unused space", "Remaining") << i18nc("Space used up", "Used")
                  << i18n("State") << i18n("Allocatable")
                  << i18n("Tags")
                  << i18n("Logical volumes");

    item = new QTreeWidgetItem((QTreeWidgetItem *)0, header_labels);

    for (int column = 0; column < 7; column++)
        item->setTextAlignment(column, Qt::AlignCenter);

    sortByColumn(0, Qt::AscendingOrder);

    item->setToolTip(0, i18n("Physical volume device"));
    item->setToolTip(1, i18n("Total size of physical volume"));
    item->setToolTip(2, i18n("Free space on physical volume"));
    item->setToolTip(3, i18n("Space used on physical volume"));
    item->setToolTip(4, i18n("A physcial volume is active if it has logical volumes that are active"));
    item->setToolTip(5, i18n("If physical volume allows more extents to be allocated"));
    item->setToolTip(6, i18n("Optional tags for physical volume"));
    item->setToolTip(7, i18n("Logical volumes on physical volume"));

    setHeaderItem(item);
    setupContextMenu();
}

void PVTree::loadData()
{
    QList<QTreeWidgetItem *> pv_tree_items;
    QString old_current_pv_name;

    if (currentItem())
        old_current_pv_name = currentItem()->data(0, 0).toString();

    clear();
    setSortingEnabled(false);
    setViewConfig();

    KLocale::BinaryUnitDialect dialect;
    KLocale *const locale = KGlobal::locale();

    if (m_use_si_units)
        dialect = KLocale::MetricBinaryDialect;
    else
        dialect = KLocale::IECBinaryDialect;

    for (auto pv : m_vg->getPhysicalVolumes()) {

        QStringList pv_data;
        const QString device_name = pv->getName();

        if (pv->isMissing()) {
            if (device_name == "unknown device")
                pv_data << i18n("MISSING");
            else
                pv_data << i18n("MISSING %1", device_name);
        } else {
            pv_data << device_name;
        }

        pv_data << locale->formatByteSize(pv->getSize(), 1, dialect);

        if (m_show_total && !m_show_percent) {
            pv_data << locale->formatByteSize(pv->getRemaining(), 1, dialect);
            pv_data << locale->formatByteSize(pv->getSize() - pv->getRemaining(), 1, dialect);
        } else if (!m_show_total && m_show_percent) {
            pv_data << QString("%%1").arg(100 - pv->getPercentUsed());
            pv_data << QString("%%1").arg(pv->getPercentUsed());
        } else if (m_show_both) {
            pv_data << QString("%1 (%%2) ").arg(locale->formatByteSize(pv->getRemaining(), 1, dialect)).arg(100 - pv->getPercentUsed());
            pv_data << QString("%1 (%%2) ").arg(locale->formatByteSize(pv->getSize() - pv->getRemaining(), 1, dialect)).arg(pv->getPercentUsed());
        }

        if (pv->isActive())
            pv_data << "Active";
        else
            pv_data << "Inactive";

        if (pv->isAllocatable())
            pv_data << "Yes";
        else
            pv_data << "No";

        pv_data << pv->getTags().join(", ");
        pv_data << getLvNames(pv).join(", ");

        QTreeWidgetItem *const item = new QTreeWidgetItem(static_cast<QTreeWidgetItem *>(nullptr), pv_data);

        if (pv->isMissing()) {
            item->setIcon(0, KIcon("exclamation"));
            item->setToolTip(0, i18n("This physical volume can not be found"));
        } else {
            item->setIcon(0, KIcon());
        }

        item->setData(0, Qt::UserRole, pv->getUuid());
        item->setData(1, Qt::UserRole, pv->getSize());
        item->setData(2, Qt::UserRole, pv->getRemaining());
        item->setData(3, Qt::UserRole, (pv->getSize() - pv->getRemaining()));

        if (m_pv_warn_percent && (m_pv_warn_percent >= (100 - pv->getPercentUsed()))) {
            item->setIcon(2, KIcon("dialog-warning"));
            item->setToolTip(2, i18n("Physical volume that is running out of space"));
            item->setIcon(3, KIcon("dialog-warning"));
            item->setToolTip(3, i18n("Physical volume that is running out of space"));
        }

        if (pv->isActive()) {
            item->setToolTip(4, i18n("Active"));
            item->setIcon(4, KIcon("lightbulb"));
        } else {
            item->setToolTip(4, i18n("Inactive"));
            item->setIcon(4, KIcon("lightbulb_off"));
        }

        for (int column = 1; column < 6; column++)
            item->setTextAlignment(column, Qt::AlignRight);

        item->setTextAlignment(6, Qt::AlignLeft);
        item->setTextAlignment(7, Qt::AlignLeft);

        pv_tree_items.append(item);
    }

    insertTopLevelItems(0, pv_tree_items);
    setSortingEnabled(true);

    for (int column = 0; column < 7; ++column) {
        if (!isColumnHidden(column))
            resizeColumnToContents(column);
    }

    if (!pv_tree_items.isEmpty() && !old_current_pv_name.isEmpty()) {
        bool match = false;
        for (int x = pv_tree_items.size() - 1; x >= 0; --x) {
            if (old_current_pv_name == pv_tree_items[x]->data(0, 0).toString()) {
                setCurrentItem(pv_tree_items[x]);
                match = true;
                break;
            }
        }
        if (!match) {
            setCurrentItem(pv_tree_items[0]);
            scrollToItem(pv_tree_items[0], QAbstractItemView::EnsureVisible);
        }
    } else if (!pv_tree_items.isEmpty()) {
        setCurrentItem(pv_tree_items[0]);
        scrollToItem(pv_tree_items[0], QAbstractItemView::EnsureVisible);
    }

    return;
}

void PVTree::setupContextMenu()
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(popupContextMenu(QPoint)));
}

void PVTree::popupContextMenu(QPoint point)
{
    emit pvMenuRequested(itemAt(point));
}

void PVTree::setViewConfig()
{
    KConfigSkeleton skeleton;

    bool pvname, pvsize,  pvremaining,
         pvused, pvstate, pvallocate,
         pvtags, pvlvnames;

    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", m_use_si_units, false);

    skeleton.setCurrentGroup("PhysicalTreeColumns");
    skeleton.addItemBool("pt_name",      pvname,      true);
    skeleton.addItemBool("pt_size",      pvsize,      true);
    skeleton.addItemBool("pt_remaining", pvremaining, true);
    skeleton.addItemBool("pt_used",      pvused,      false);
    skeleton.addItemBool("pt_state",     pvstate,     false);
    skeleton.addItemBool("pt_allocate",  pvallocate,  true);
    skeleton.addItemBool("pt_tags",      pvtags,      true);
    skeleton.addItemBool("pt_lvnames",   pvlvnames,   true);

    skeleton.setCurrentGroup("AllTreeColumns");
    skeleton.addItemBool("show_total",   m_show_total,   false);
    skeleton.addItemBool("show_percent", m_show_percent, false);
    skeleton.addItemBool("show_both",    m_show_both,    true);
    skeleton.addItemInt("pv_warn", m_pv_warn_percent,  0);

    if (!(!pvname == isColumnHidden(0)      && !pvsize == isColumnHidden(1) &&
          !pvremaining == isColumnHidden(2) && !pvused == isColumnHidden(3) &&
          !pvstate == isColumnHidden(4)     && !pvallocate == isColumnHidden(5) &&
          !pvtags == isColumnHidden(6)      && !pvlvnames == isColumnHidden(7))) {

        setColumnHidden(0, !pvname);
        setColumnHidden(1, !pvsize);
        setColumnHidden(2, !pvremaining);
        setColumnHidden(3, !pvused);
        setColumnHidden(4, !pvstate);
        setColumnHidden(5, !pvallocate);
        setColumnHidden(6, !pvtags);
        setColumnHidden(7, !pvlvnames);
    }
}


/* here we get the names of logical volumes associated
   With the physical volume */

QStringList PVTree::getLvNames(PhysVol *const pv)
{
    const QString current_name = pv->getName();
    QStringList lv_names;
    
    for (auto lv : m_vg->getLogicalVolumesFlat()) {
        for (auto pv_name : lv->getPvNamesAll()) {
            if (current_name == pv_name)
                lv_names << lv->getName();
        }
    }
    
    lv_names.sort();
    lv_names.removeDuplicates();
    
    return lv_names;
}
