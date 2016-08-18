/*
 *
 *
 * Copyright (C) 2011, 2012, 2013, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 * * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "devicetree.h"

#include <KConfigSkeleton>
#include <KFormat>
#include <KLocalizedString>

#include <QIcon>

#include "devicemenu.h"
#include "devicepropertiesstack.h"
#include "devicesizechart.h"
#include "physvol.h"
#include "storagedevice.h"
#include "storagepartition.h"
#include "volgroup.h"


DeviceTree::DeviceTree(DeviceSizeChart *const chart, DevicePropertiesStack *const stack, QWidget *parent)
    : QTreeWidget(parent),
      m_chart(chart),
      m_stack(stack)
{
    const QStringList headers = QStringList() << "Device"    << "Type"  << "Capacity"
                                              << "Remaining" << "Usage" << "Group"
                                              << "Flags"     << "Mount point" ;

    QTreeWidgetItem *const item = new QTreeWidgetItem(static_cast<QTreeWidgetItem *>(nullptr), headers);

    for (int column = 0; column < item->columnCount() ; column++)
        item->setTextAlignment(column, Qt::AlignCenter);

    item->setToolTip(0, i18n("The device name"));
    item->setToolTip(1, i18n("The type of partition or device"));
    item->setToolTip(2, i18n("The amount of storage space"));
    item->setToolTip(3, i18n("The remaining storage space"));
    item->setToolTip(4, i18n("How the device is being used"));
    item->setToolTip(5, i18n("The Name of the volume group if the device is a physical volume"));
    item->setToolTip(6, i18n("Any flags associated with device"));
    item->setToolTip(7, i18n("Mount points of the filesystem if it is mounted"));

    m_initial_run = true;
    setHeaderItem(item);
    setAlternatingRowColors(true);
    setAllColumnsShowFocus(true);
    setExpandsOnDoubleClick(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setupContextMenu();
}

    /*
       item->data(x, Qt::UserRole)
       x = 0:  pointer to storagepartition if partition, else ""
       x = 1:  pointer to storagedevice
    */


void DeviceTree::loadData(QList<StorageDevice *> devices)
{
    setSortingEnabled(false);

    disconnect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem *)),
               m_chart, SLOT(setNewDevice(QTreeWidgetItem*)));

    disconnect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
               m_stack, SLOT(changeDeviceStackIndex(QTreeWidgetItem*)));

    setViewConfig();

    QString current_device, current_parent;
    currentItemNames(current_device, current_parent); // Note - pass by reference, returns both args
    QStringList expanded_items, old_dev_names;
    expandedItemNames(expanded_items, old_dev_names); // Ditto

    for (auto dev : devices) {
        QTreeWidgetItem  *const dev_item = buildDeviceItem(dev);
        addTopLevelItem(dev_item);

        for (auto const part : dev->getStoragePartitions()) {
            if (!part->isLogical() && !part->isLogicalFreespace()) {
                QTreeWidgetItem  *const part_item = buildPartitionItem(part, dev);
                dev_item->addChild(part_item);

                if (part->isExtended())
                    expandItem(part_item, expanded_items, old_dev_names);
            }
        }

        expandItem(dev_item, expanded_items, old_dev_names);
    }

    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem *)),
            m_chart, SLOT(setNewDevice(QTreeWidgetItem*)));

    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
            m_stack, SLOT(changeDeviceStackIndex(QTreeWidgetItem*)));

    restoreCurrentItem(current_device, current_parent);

    resizeColumnToContents(0);
    resizeColumnToContents(3);
    resizeColumnToContents(5);
}

void DeviceTree::setupContextMenu()
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(popupContextMenu(QPoint)));
}

void DeviceTree::popupContextMenu(QPoint point)
{
    emit deviceMenuRequested(itemAt(point));
}

void DeviceTree::setViewConfig()
{
    KConfigSkeleton skeleton;

    bool device,
         partition,
         capacity,
         remaining,
         usage,
         group,
         flags,
         mount;

    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", m_use_si_units, false);

    skeleton.setCurrentGroup("DeviceTreeColumns");
    skeleton.addItemBool("dt_device",      device,    true);
    skeleton.addItemBool("dt_partition",   partition, true);
    skeleton.addItemBool("dt_capacity",    capacity,  true);
    skeleton.addItemBool("dt_remaining",   remaining, true);
    skeleton.addItemBool("dt_usage",       usage,     true);
    skeleton.addItemBool("dt_group",       group,     true);
    skeleton.addItemBool("dt_flags",       flags,     true);
    skeleton.addItemBool("dt_mount",       mount,     true);
    skeleton.addItemBool("dt_expandparts", m_expand_parts, true);

    skeleton.setCurrentGroup("AllTreeColumns");
    skeleton.addItemBool("show_total",   m_show_total,   false);
    skeleton.addItemBool("show_percent", m_show_percent, false);
    skeleton.addItemBool("show_both",    m_show_both,    true);
    skeleton.addItemInt("fs_warn", m_fs_warn_percent, 10);
    skeleton.addItemInt("pv_warn", m_pv_warn_percent,  0);

    bool hidden_columns_changed = false;

    if (isColumnHidden(0) == device) {
        hidden_columns_changed = true;
        setColumnHidden(0, !device);
    }

    if (isColumnHidden(1) == partition) {
        hidden_columns_changed = true;
        setColumnHidden(1, !partition);
    }

    if (isColumnHidden(2) == capacity) {
        hidden_columns_changed = true;
        setColumnHidden(2, !capacity);
    }

    if (isColumnHidden(3) == remaining) {
        hidden_columns_changed = true;
        setColumnHidden(3, !remaining);
    }

    if (isColumnHidden(4) == usage) {
        hidden_columns_changed = true;
        setColumnHidden(4, !usage);
    }

    if (isColumnHidden(5) == group) {
        hidden_columns_changed = true;
        setColumnHidden(5, !group);
    }

    if (isColumnHidden(6) == flags) {
        hidden_columns_changed = true;
        setColumnHidden(6, !flags);
    }

    if (isColumnHidden(7) == mount) {
        hidden_columns_changed = true;
        setColumnHidden(7, !mount);
    }

    bool skip = true;
    if (hidden_columns_changed) {
        for (int i = 7; i >= 0; --i) {
            if (!isColumnHidden(i)) {
                if (skip) {
                    skip = false;
                } else {
                    resizeColumnToContents(i);
                }
            }
        }
    }
}

QTreeWidgetItem *DeviceTree::buildDeviceItem(StorageDevice *const dev)
{
    QTreeWidgetItem *const item = new QTreeWidgetItem(getDeviceItemData(dev));
    QVariant dev_variant;

    dev_variant.setValue(static_cast<void *>(dev));
    item->setData(1, Qt::UserRole, dev_variant);
    item->setTextAlignment(2, Qt::AlignRight);
    item->setTextAlignment(3, Qt::AlignRight);

    setItemAttributes(item, dev);
    
    return item;
}

QTreeWidgetItem *DeviceTree::buildPartitionItem(StoragePartition *const part, StorageDevice *const dev) 
{
    QTreeWidgetItem *const item = new QTreeWidgetItem(getPartitionItemData(part));

    QVariant part_variant, dev_variant;
    part_variant.setValue(static_cast<void *>(part));
    dev_variant.setValue(static_cast<void *>(dev));

    item->setData(0, Qt::UserRole, part_variant);
    item->setData(1, Qt::UserRole, dev_variant);
    item->setTextAlignment(2, Qt::AlignRight);
    item->setTextAlignment(3, Qt::AlignRight);

    setItemAttributes(item, part);

    if (part->isExtended()) {
        for (auto part : dev->getStoragePartitions()) {
            if (part->isLogical() || part->isLogicalFreespace())
                item->addChild(buildPartitionItem(part, dev));
        }
    }
    
    return item;
}

QStringList DeviceTree::getDeviceItemData(const StorageDevice *const dev)
{
    KFormat::BinaryUnitDialect dialect;

    if (m_use_si_units)
        dialect = KFormat::MetricBinaryDialect;
    else
        dialect = KFormat::IECBinaryDialect;

    QString external_raid;
    if (dev->isDmRaid())
        external_raid = "dm volume";
    else if (dev->isMdRaid())
        external_raid = "md volume";

    QStringList data = QStringList() << dev->getName() << external_raid;

    if (dev->isPhysicalVolume()) {
        data << KFormat().formatByteSize(dev->getSize(), 1, dialect);
        const PhysVol *const pv = dev->getPhysicalVolume();
        
        if (m_show_total && !m_show_percent)
            data << KFormat().formatByteSize(pv->getRemaining(), 1, dialect);
        else if (!m_show_total && m_show_percent)
            data << QString("%%1").arg(100 - pv->getPercentUsed());
        else if (m_show_both)
            data << QString("%1 (%%2) ").arg(KFormat().formatByteSize(pv->getRemaining(), 1, dialect)).arg(100 - pv->getPercentUsed());
        
        data << "PV" << pv->getVg()->getName();
    } else {
        data << KFormat().formatByteSize(dev->getSize(), 1, dialect) << "";
        if(dev->isDmBlock())
            data << "dm device";
        else if(dev->isMdBlock())
            data << "md device";
    }

    return data;
}

QStringList DeviceTree::getPartitionItemData(const StoragePartition *const part) 
{
    KFormat::BinaryUnitDialect dialect;

    if (m_use_si_units)
        dialect = KFormat::MetricBinaryDialect;
    else
        dialect = KFormat::IECBinaryDialect;

    QStringList data = QStringList() << part->getName() << part->getType() << KFormat().formatByteSize(part->getSize(), 1, dialect);
    
    if (part->isPhysicalVolume()) {
        PhysVol *const pv = part->getPhysicalVolume();
        
        if (m_show_total && !m_show_percent)
            data << KFormat().formatByteSize(pv->getRemaining(), 1, dialect);
        else if (!m_show_total && m_show_percent)
            data << QString("%%1").arg(100 - pv->getPercentUsed());
        else
            data << QString("%1 (%%2) ").arg(KFormat().formatByteSize(pv->getRemaining(), 1, dialect)).arg(100 - pv->getPercentUsed());
        
        data << "PV"
             << pv->getVg()->getName()
             << (part->getFlags()).join(", ")
             << "";
    } else {
        if (part->getFilesystemSize() > -1 && part->getFilesystemUsed() > -1) {
            
            if (m_show_total && !m_show_percent)
                data << KFormat().formatByteSize(part->getFilesystemRemaining(), 1, dialect);
            else if (!m_show_total && m_show_percent)
                data << QString("%%1").arg(100 - part->getFilesystemPercentUsed());
            else
                data << QString("%1 (%%2) ").arg(KFormat().formatByteSize(part->getFilesystemRemaining(), 1, dialect)).arg(100 - part->getFilesystemPercentUsed());
        } else {
            data << "";
        }

        if(part->isMdBlock())
            data << "md device";
        else
            data << part->getFilesystem();
        
        data << "" << (part->getFlags()).join(", ");
        
        if (part->isMounted())
            data << (part->getMountPoints())[0];
        else if (part->isBusy() && (part->getFilesystem() == "swap"))
            data << "swapping";
        else
            data << "";
    }

    return data;
}


// Re-sets the current item back to what it was, if possible.
// If the old current item is gone it looks for the parent of the
// current item next.

void DeviceTree::restoreCurrentItem(const QString current, const QString currentParent)
{
    bool match = false;

    if (m_initial_run) {
        m_initial_run = false;
        if (m_expand_parts)
            expandAll();
    } else {
        for (int i = topLevelItemCount() - 1; i >= 0; --i) {
            QTreeWidgetItem *const parent = topLevelItem(i);

            if (parent->data(0, Qt::DisplayRole).toString() == current) {
                match = true;
                setCurrentItem(parent);
                break;
            }
            for (int j = parent->childCount() - 1; j >= 0; --j) {
                QTreeWidgetItem *const child = parent->child(j);
                if (child->data(0, Qt::DisplayRole).toString() == current) {
                    match = true;
                    setCurrentItem(child);
                    break;
                }
                for (int k = child->childCount() - 1; k >= 0; --k) {
                    if (child->child(k)->data(0, Qt::DisplayRole).toString() == current) {
                        match = true;
                        setCurrentItem(child->child(k));
                        break;
                    }
                }
                if ((child->data(0, Qt::DisplayRole).toString() == currentParent) && (!match)) {
                    match = true;
                    setCurrentItem(child);
                    break;
                }
                if (match)
                    break;
            }
            if ((parent->data(0, Qt::DisplayRole).toString() == currentParent) && (!match)) {
                match = true;
                setCurrentItem(parent);
                break;
            }
            if (match)
                break;
        }
    }

    if (topLevelItemCount() && (currentItem() == nullptr))
        setCurrentItem(topLevelItem(0));
}


// Gets the name of the current item (the highlighted one) in the tree
// and the name of its parent.

void DeviceTree::currentItemNames(QString &current, QString &currentParent)
{
    if (currentItem()){
        current = currentItem()->data(0, Qt::DisplayRole).toString();

        if (currentItem()->parent())
            currentParent = currentItem()->parent()->data(0, Qt::DisplayRole).toString();
        else
            currentParent = current;
    } else {
        current.clear();
        currentParent.clear();
    }
}


// Gets the names of just the expanded items and the names of all items
// in the tree before everything gets deleted.

void DeviceTree::expandedItemNames(QStringList &expanded, QStringList &old)
{
    expanded.clear();
    old.clear();

    for (int i = topLevelItemCount() - 1; i >= 0; --i) {
        auto parent = topLevelItem(i);

        if (parent->isExpanded())
            expanded << parent->data(0, Qt::DisplayRole).toString();

        for (int j = parent->childCount() - 1; j >= 0; --j) {
            auto child = parent->child(j);

            if (child->isExpanded())
                expanded << child->data(0, Qt::DisplayRole).toString();
            old << child->data(0, Qt::DisplayRole).toString();
        }

        old << parent->data(0, Qt::DisplayRole).toString();
        delete takeTopLevelItem(i);
    }
}


// If the old item for this dev was expanded, expand new one. Also
// expand it if it didn't exsist before and m_expand_part config is set.

void DeviceTree::expandItem(QTreeWidgetItem *const item, const QStringList expanded, const QStringList old)
{
    const QString name = item->data(0, Qt::DisplayRole).toString();

    if (old.contains(name)) {
        if(expanded.contains(name))
            item->setExpanded(true);
        else
            item->setExpanded(false);
    } else {
        item->setExpanded(m_expand_parts);
    }
}

void DeviceTree::setItemAttributes(QTreeWidgetItem *const item, const StorageBase *const devbase)
{
    if (devbase->isDmRaid())
        item->setToolTip(1, i18n("A device mapper RAID volume"));
    else if (devbase->isMdRaid())
        item->setToolTip(1, i18n("A multiple device RAID volume"));

    if (devbase->isDmBlock()) {
        item->setToolTip(4, i18n("A block device under a device mapper RAID volume"));
    } else if (devbase->isMdBlock()) {
        item->setToolTip(4, i18n("A block device under a multiple device RAID volume"));
    } if (devbase->isPhysicalVolume()) {
        if (m_pv_warn_percent && (m_pv_warn_percent >= (100 - devbase->getPhysicalVolume()->getPercentUsed()))) {
            item->setIcon(3, QIcon::fromTheme(QStringLiteral("dialog-warning")));
            item->setToolTip(3, i18n("Physical volume that is running out of space"));
        }
        
        if (devbase->getPhysicalVolume()->isActive()) {
            item->setIcon(4, QIcon::fromTheme(QStringLiteral("lightbulb")));
            item->setToolTip(4, i18n("Physical volume with active logical volumes"));
        } else {
            item->setIcon(4, QIcon::fromTheme(QStringLiteral("lightbulb_off")));
            item->setToolTip(4, i18n("Physical volume without active logical volumes"));
        }
    }

    const StoragePartition *const part = dynamic_cast<const StoragePartition *>(devbase);

    if(part) {  // will be null pointer here if it is really a device, not a partition.
        if (part->isMountable()) {
            if (part->isMounted()) {
                if (m_fs_warn_percent && (m_fs_warn_percent >= (100 - part->getFilesystemPercentUsed()))) {
                    item->setIcon(3, QIcon::fromTheme(QStringLiteral("dialog-warning")));
                    item->setToolTip(3, i18n("Filesystem that is running out of space"));
                }
                
                item->setIcon(4, QIcon::fromTheme(QStringLiteral("emblem-mounted")));
                item->setToolTip(4, i18n("mounted filesystem"));
            } else {
                item->setIcon(4, QIcon::fromTheme(QStringLiteral("emblem-unmounted")));
                item->setToolTip(4, i18n("unmounted filesystem"));
            }
        } else if (part->getFilesystem() == "swap") {
            if (part->isBusy()) {
                item->setIcon(4, QIcon::fromTheme(QStringLiteral("task-recurring")));
                item->setToolTip(4, i18n("Active swap area"));
            } else {
                item->setIcon(4, QIcon::fromTheme(QStringLiteral("emblem-unmounted")));
                item->setToolTip(4, i18n("Inactive swap area"));
            }
        }
    }
}
