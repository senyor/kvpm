/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 * * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "vgtree.h"

#include <KGlobal>
#include <KConfigSkeleton>
#include <KIcon>
#include <KLocale>

#include <QtGui>

#include "lvactionsmenu.h"
#include "logvol.h"
#include "volgroup.h"


VGTree::VGTree(VolGroup *const group) : QTreeWidget(), m_vg(group)
{
    m_init = true;

    const QStringList headers = QStringList() << i18n("Volume")      << i18n("type")       << i18n("Size")
                                << i18n("Remaining")   << i18n("Filesystem") << i18n("Stripes")
                                << i18n("Stripe size") << i18n("Snap/Copy")  << i18n("State")
                                << i18n("Access")      << i18n("Tags")       << i18n("Mount points");

    QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidgetItem *)0, headers);

    for (int column = 0; column < item->columnCount() ; column++)
        item->setTextAlignment(column, Qt::AlignCenter);

    item->setToolTip(0, i18n("Logical volume name"));
    item->setToolTip(1, i18n("Type of logical volume"));
    item->setToolTip(2, i18n("Total size of the logical volume"));
    item->setToolTip(3, i18n("Free space on logical volume"));
    item->setToolTip(4, i18n("Filesystem type on logical volume, if any"));
    item->setToolTip(5, i18n("Number of stripes if the volume is striped"));
    item->setToolTip(6, i18n("Size of stripes if the volume is striped"));
    item->setToolTip(7, i18n("Percentage of pvmove completed, of mirror synced or of snapshot used up"));
    item->setToolTip(8, i18n("Logical volume state"));
    item->setToolTip(9, i18n("Read and write or Read Only"));
    item->setToolTip(10, i18n("Optional tags for physical volume"));
    item->setToolTip(11, i18n("Filesystem mount points, if mounted"));

    setHeaderItem(item);
    sortByColumn(0, Qt::AscendingOrder);
}

void VGTree::loadData()
{
    QList<LogVol *> logical_volumes = m_vg->getLogicalVolumes();
    LogVol *lv = NULL;
    QTreeWidgetItem *new_item;

    disconnect(this, SIGNAL(itemExpanded(QTreeWidgetItem *)),
               this, SLOT(adjustColumnWidth(QTreeWidgetItem *)));

    disconnect(this, SIGNAL(itemCollapsed(QTreeWidgetItem *)),
               this, SLOT(adjustColumnWidth(QTreeWidgetItem *)));

    setSortingEnabled(false);
    setViewConfig();

    for (int x = 0; x < m_vg->getLvCount(); x++) {

        lv = logical_volumes[x];
        new_item = NULL;

        for (int y = topLevelItemCount() - 1; y >= 0; y--) {
            if (topLevelItem(y)->data(0, Qt::DisplayRole).toString() == lv->getName())
                new_item = loadItem(lv, topLevelItem(y));
        }

        if (new_item == NULL) {
            new_item = new QTreeWidgetItem((QTreeWidgetItem *)0);
            addTopLevelItem(new_item);
            loadItem(lv, new_item);
        }

    }

    for (int y = topLevelItemCount() - 1; y >= 0; y--) { // remove top level lv items of deleted lvs
        bool match = false;
        for (int x = 0; x < logical_volumes.size(); x++) {

            if (topLevelItem(y)->data(0, Qt::DisplayRole).toString() == logical_volumes[x]->getName())
                match = true;
        }
        if (!match) {
            delete takeTopLevelItem(y);
        }
    }

    connect(this, SIGNAL(itemExpanded(QTreeWidgetItem *)),
            this, SLOT(adjustColumnWidth(QTreeWidgetItem *)));

    connect(this, SIGNAL(itemCollapsed(QTreeWidgetItem *)),
            this, SLOT(adjustColumnWidth(QTreeWidgetItem *)));

    setSortingEnabled(true);

    if (currentItem() == NULL && topLevelItemCount() > 0)
        setCurrentItem(topLevelItem(0));

    if (currentItem() != NULL) {
        setCurrentItem(currentItem());
        scrollToItem(currentItem(), QAbstractItemView::EnsureVisible);
    }

    for (int x = 0; x < 10; x++)
        resizeColumnToContents(x);

    setupContextMenu();
    m_init = false;
    emit currentItemChanged(currentItem(), currentItem());

    return;
}

QTreeWidgetItem *VGTree::loadItem(LogVol *lv, QTreeWidgetItem *item)
{
    const QString old_type = item->data(1, Qt::DisplayRole).toString();  // lv type before reload or "" if new item
    const QString lv_name = lv->getName();
    const bool was_sc = old_type.contains("origin", Qt::CaseInsensitive);
    const bool is_sc  = lv->isSnapContainer();
    const int old_child_count = item->childCount();
    bool was_expanded = false;

    KLocale *const locale = KGlobal::locale();
    if (m_use_si_units)
        locale->setBinaryUnitDialect(KLocale::MetricBinaryDialect);
    else
        locale->setBinaryUnitDialect(KLocale::IECBinaryDialect);

    QList<LogVol *> temp_kids;
    long long fs_remaining;       // remaining space on fs -- if known
    int fs_percent;               // percentage of space remaining

    /*
      UserRole definitions:

      setData(0, Qt::UserRole, lv->getName());
      setData(1, Qt::UserRole, segment);            // segment number
      setData(2, Qt::UserRole, lv->getUuid());
      setData(3, Qt::UserRole, QString("segment")); // "" if not a segment item
      setData(4, Qt::UserRole, bool);               // true if it is a snap container
    */


    if (is_sc && !was_sc)
        was_expanded = item->isExpanded();

    if (!is_sc && was_sc) {
        for (int x = 0; x < old_child_count; x++) {
            if (lv_name == item->child(x)->data(0, Qt::DisplayRole).toString())
                was_expanded = item->child(x)->isExpanded();
        }
    }

    if (is_sc)
        item->setData(4, Qt::UserRole, true);
    else
        item->setData(4, Qt::UserRole, false);

    item->setData(0, Qt::DisplayRole, lv_name);

    if (lv->hasMissingVolume()) {
        item->setIcon(0, KIcon("exclamation"));
        item->setToolTip(0, i18n("one or more physical volumes are missing"));
    } else if (!lv->isSnapContainer() && lv->isOrigin()) {
        item->setIcon(0, KIcon("bullet_star"));
        item->setToolTip(0, i18n("origin"));
    } else {
        item->setIcon(0, KIcon());
        item->setToolTip(0, QString());
    }

    item->setData(1, Qt::DisplayRole, lv->getType());

    if (lv->isSnapContainer()) {
        item->setData(2, Qt::DisplayRole, locale->formatByteSize(lv->getTotalSize()));

        for (int x = 3; x < 12; x++)
            item->setData(x, Qt::DisplayRole, QVariant());

        item->setIcon(3, KIcon());
        item->setToolTip(3, QString());
        item->setIcon(8, KIcon());
        item->setToolTip(8, QString());
    } else {
        item->setData(2, Qt::DisplayRole, locale->formatByteSize(lv->getSize()));

        if (lv->getFilesystemSize() > -1 &&  lv->getFilesystemUsed() > -1) {

            fs_remaining = lv->getFilesystemSize() - lv->getFilesystemUsed();
            fs_percent = qRound(((double)fs_remaining / (double)lv->getFilesystemSize()) * 100);

            if (m_show_total)
                item->setData(3, Qt::DisplayRole, locale->formatByteSize(fs_remaining));
            else if (m_show_percent)
                item->setData(3, Qt::DisplayRole, QString("%%1").arg(fs_percent));
            else if (m_show_both)
                item->setData(3, Qt::DisplayRole, QString(locale->formatByteSize(fs_remaining) + " (%%1)").arg(fs_percent));

            if (fs_percent <= m_fs_warn_percent) {
                item->setIcon(3, KIcon("exclamation"));
                item->setToolTip(3, i18n("This filesystem is running out of space"));
            } else {
                item->setIcon(3, KIcon());
                item->setToolTip(3, QString());
            }
        } else {
            item->setData(3, Qt::DisplayRole, QString(""));
            item->setIcon(3, KIcon());
            item->setToolTip(3, QString());
        }

        item->setData(4, Qt::DisplayRole, lv->getFilesystem());

        if ((lv->isPvmove() || lv->isMirror()) && !lv->isSnapContainer())
            item->setData(7, Qt::DisplayRole, QString("%%1").arg(lv->getCopyPercent(), 1, 'f', 2));
        else if (lv->isSnap() || lv->isMerging())
            item->setData(7, Qt::DisplayRole, QString("%%1").arg(lv->getSnapPercent(), 1, 'f', 2));
        else
            item->setData(7, Qt::DisplayRole, QString(""));

        item->setData(8, Qt::DisplayRole, lv->getState());

        if (!lv->isSnapContainer() && !lv->isMirrorLog() && !lv->isMirrorLeg() && !lv->isVirtual()) {
            if (lv->isMounted()) {
                item->setIcon(8, KIcon("emblem-mounted"));
                item->setToolTip(8, i18n("mounted filesystem"));
            } else if (lv->getFilesystem() == "swap") {
                if (lv->isOpen()) {
                    item->setIcon(8, KIcon("task-recurring"));
                    item->setToolTip(8, i18n("Active swap area"));
                } else {
                    item->setIcon(8, KIcon("emblem-unmounted"));
                    item->setToolTip(8, i18n("Inactive swap area"));
                }
            } else {
                item->setIcon(8, KIcon("emblem-unmounted"));
                item->setToolTip(8, i18n("unmounted filesystem"));
            }
        }

        if (lv->isWritable())
            item->setData(9, Qt::DisplayRole, QString("r/w"));
        else
            item->setData(9, Qt::DisplayRole, QString("r/o"));

        item->setData(10, Qt::DisplayRole, lv->getTags().join(","));
        item->setData(11, Qt::DisplayRole, lv->getMountPoints().join(","));
    }

    item->setData(0, Qt::UserRole, lv_name);
    item->setData(2, Qt::UserRole, lv->getUuid());

    if (lv->getSegmentCount() == 1) {
        item->setData(1, Qt::UserRole, 0);            // 0 means segment 0 data present

        if (lv->isMirror()) {
            item->setData(5, Qt::DisplayRole, QString(""));
            item->setData(6, Qt::DisplayRole, QString(""));
        } else {
            item->setData(5, Qt::DisplayRole, QString("%1").arg(lv->getSegmentStripes(0)));
            item->setData(6, Qt::DisplayRole, locale->formatByteSize(lv->getSegmentStripeSize(0)));
        }

        if (!is_sc && old_type.contains("origin", Qt::CaseInsensitive)) {
            for (int x = 0; x < old_child_count; x++) {
                if (item->child(x)->data(0, Qt::DisplayRole) == lv->getName())
                    item->setExpanded(item->child(x)->isExpanded());
            }
        }

        insertChildItems(lv, item);
    } else {
        item->setData(1, Qt::UserRole, -1);            // -1 means not segment data
        item->setData(5, Qt::DisplayRole, QString(""));
        item->setData(6, Qt::DisplayRole, QString(""));

        insertSegmentItems(lv, item);
    }

    const int new_child_count = item->childCount();

    if (is_sc) { // expand the item if it is a new snap container or snap count is different

        if (m_init) {
            item->setExpanded(true);
        } else {
            if (!was_sc || old_child_count != new_child_count) {
                item->setExpanded(true);
                if (!was_sc) {
                    for (int x = 0; x < new_child_count; x++) {
                        if (item->child(x)->data(0, Qt::DisplayRole) == lv_name)
                            item->child(x)->setExpanded(was_expanded);
                    }
                }
            }
        }
    } else if (was_sc && !is_sc) { // if it was formerly a snap container, set expanded to what the "real" lv was
        if (indexOfTopLevelItem(item) >= 0)
            addTopLevelItem(takeTopLevelItem(indexOfTopLevelItem(item)));     // next line doesn't work without this!
        item->setExpanded(was_expanded);
    }

    for (int column = 1; column < item->columnCount() ; column++)
        item->setTextAlignment(column, Qt::AlignRight);

    return item;
}


void VGTree::setupContextMenu()
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    // disconnect the last connect, otherwise the following connect get repeated
    // and piles up.

    disconnect(this, SIGNAL(customContextMenuRequested(QPoint)),
               this, SLOT(popupContextMenu(QPoint)));

    if (!m_vg->isExported()) {

        connect(this, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(popupContextMenu(QPoint)));

    }

    return;
}

void VGTree::insertChildItems(LogVol *parentVolume, QTreeWidgetItem *parentItem)
{
    LogVol *child_volume;
    const QList<LogVol *>  immediate_children = parentVolume->getChildren();

    for (int x = 0; x < immediate_children.size(); x++) {

        QTreeWidgetItem *child_item = NULL;
        child_volume = immediate_children[x];

        for (int y = parentItem->childCount() - 1; y >= 0; y--) {
            if (parentItem->child(y)->data(0, Qt::DisplayRole).toString() == child_volume->getName())
                child_item = loadItem(child_volume, parentItem->child(y));
        }

        if (child_item == NULL)
            child_item = loadItem(child_volume, new QTreeWidgetItem(parentItem));

        for (int column = 1; column < child_item->columnCount() ; column++)
            child_item->setTextAlignment(column, Qt::AlignRight);
    }

    // Remove child items for logical volumes that no longer exist
    for (int y = parentItem->childCount() - 1; y >= 0; y--) {

        bool match = false;

        for (int x = 0; x < immediate_children.size(); x++) {
            child_volume = immediate_children[x];

            if (parentItem->child(y)->data(0, Qt::DisplayRole).toString() == child_volume->getName())
                match = true;
        }

        if (!match)
            delete parentItem->takeChild(y);
    }

    return;
}

void VGTree::popupContextMenu(QPoint point)
{
    KMenu *context_menu = NULL;
    const QTreeWidgetItem *const item = itemAt(point);

    if (item) {                               //item = 0 if there is no item a that point

        LogVol *const lv = m_vg->getLvByName(QVariant(item->data(0, Qt::UserRole)).toString());
        int segment;    // segment = -1 means whole lv

        if (QVariant(item->data(3, Qt::UserRole)).toString() == "segment")
            segment = QVariant(item->data(1, Qt::UserRole)).toInt();
        else
            segment = -1;

        context_menu = new LVActionsMenu(lv, segment, m_vg, this);
        context_menu->exec(QCursor::pos());
    } else {
        context_menu = new LVActionsMenu(NULL, 0, m_vg, this);
        context_menu->exec(QCursor::pos());
    }

    if (context_menu != NULL)
        context_menu->deleteLater();
}

void VGTree::setViewConfig()
{
    KConfigSkeleton skeleton;

    bool changed = false;

    bool volume,      size,      remaining,
         filesystem,  type,
         stripes,     stripesize,
         snapmove,    state,
         access,      tags,
         mountpoints;

    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", m_use_si_units, false);

    skeleton.setCurrentGroup("AllTreeColumns");
    skeleton.addItemBool("show_percent", m_show_percent, false);
    skeleton.addItemBool("show_total",   m_show_total,   false);
    skeleton.addItemBool("show_both",    m_show_both,    true);
    skeleton.addItemInt("fs_warn", m_fs_warn_percent, 10);

    skeleton.setCurrentGroup("VolumeTreeColumns");
    skeleton.addItemBool("vt_volume",      volume,      true);
    skeleton.addItemBool("vt_size",        size,        true);
    skeleton.addItemBool("vt_remaining",   remaining,   true);
    skeleton.addItemBool("vt_type",        type,        true);
    skeleton.addItemBool("vt_filesystem",  filesystem,  false);
    skeleton.addItemBool("vt_stripes",     stripes,     false);
    skeleton.addItemBool("vt_stripesize",  stripesize,  false);
    skeleton.addItemBool("vt_snapmove",    snapmove,    true);
    skeleton.addItemBool("vt_state",       state,       true);
    skeleton.addItemBool("vt_access",      access,      false);
    skeleton.addItemBool("vt_tags",        tags,        true);
    skeleton.addItemBool("vt_mountpoints", mountpoints, false);

    if (!(!volume == isColumnHidden(0)     && !size == isColumnHidden(1) &&
            !remaining == isColumnHidden(2)  && !filesystem == isColumnHidden(3) &&
            !type == isColumnHidden(4)       && !stripes == isColumnHidden(5) &&
            !stripesize == isColumnHidden(6) && !snapmove == isColumnHidden(7) &&
            !state == isColumnHidden(8)      && !access == isColumnHidden(9) &&
            !tags == isColumnHidden(10)      && !mountpoints == isColumnHidden(11)))
        changed = true;

    if (changed) {

        setColumnHidden(0, !volume);
        setColumnHidden(1, !type);
        setColumnHidden(2, !size);
        setColumnHidden(3, !remaining);
        setColumnHidden(4, !filesystem);
        setColumnHidden(5, !stripes);
        setColumnHidden(6, !stripesize);
        setColumnHidden(7, !snapmove);
        setColumnHidden(8, !state);
        setColumnHidden(9, !access);
        setColumnHidden(10, !tags);
        setColumnHidden(11, !mountpoints);

        for (int column = 0; column < 11; column++) {
            if (!isColumnHidden(column))
                resizeColumnToContents(column);
        }
    }
}

void VGTree::adjustColumnWidth(QTreeWidgetItem *)
{
    resizeColumnToContents(0);
    resizeColumnToContents(1);
    resizeColumnToContents(6);
}

void VGTree::insertSegmentItems(LogVol *lv, QTreeWidgetItem *item)
{
    const int segment_count = lv->getSegmentCount();
    const int child_count = item->childCount();

    KLocale *const locale = KGlobal::locale();
    if (m_use_si_units)
        locale->setBinaryUnitDialect(KLocale::MetricBinaryDialect);
    else
        locale->setBinaryUnitDialect(KLocale::IECBinaryDialect);

    QTreeWidgetItem *child_item;
    QList<QTreeWidgetItem *> segment_children;

    for (int x = 0; x < child_count ; x++) // segments can never have children
        segment_children.append(item->child(x)->takeChildren());

    for (int x = segment_children.size() - 1; x >= 0 ; x--)
        delete segment_children[x]; // so delete them

    if (segment_count > child_count) {
        for (int x = 0; x < segment_count - child_count; x++)
            new QTreeWidgetItem(item);
    } else if (segment_count < child_count) {
        for (int x = child_count - 1; x >= segment_count ; x--)
            delete(item->takeChild(x));
    }

    for (int segment = 0; segment < segment_count; segment++) {

        child_item = item->child(segment);

        if (lv->getPvNames(segment).contains("unknown device")) {
            child_item->setIcon(0, KIcon("exclamation"));
            child_item->setToolTip(0, i18n("one or more physical volumes are missing"));
        } else {
            child_item->setIcon(0, KIcon());
            child_item->setToolTip(0, QString());
        }

        child_item->setData(0, Qt::DisplayRole, QString("Seg# %1").arg(segment));
        child_item->setData(1, Qt::DisplayRole, QString(""));
        child_item->setData(2, Qt::DisplayRole, locale->formatByteSize(lv->getSegmentSize(segment)));
        child_item->setData(3, Qt::DisplayRole, QString(""));
        child_item->setData(4, Qt::DisplayRole, QString(""));
        child_item->setData(5, Qt::DisplayRole, QString("%1").arg(lv->getSegmentStripes(segment)));
        child_item->setData(6, Qt::DisplayRole, locale->formatByteSize(lv->getSegmentStripeSize(segment)));
        child_item->setData(0, Qt::UserRole, lv->getName());
        child_item->setData(1, Qt::UserRole, segment);
        child_item->setData(2, Qt::UserRole, lv->getUuid());
        child_item->setData(3, Qt::UserRole, QString("segment"));

        for (int column = 7; column < 12; column++)
            child_item->setData(column, Qt::DisplayRole, QString(""));

        for (int column = 1; column < child_item->columnCount() ; column++)
            child_item->setTextAlignment(column, Qt::AlignRight);
    }
}

