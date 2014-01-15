/*
 *
 *
 * Copyright (C) 2008, 2009, 2011, 2012, 2013, 2014 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "devicesizechart.h"

#include "devicesizechartseg.h"
#include "devicetree.h"
#include "storagedevice.h"
#include "storagepartition.h"

#include <parted/disk.h>

#include <KSeparator>

#include <QDebug>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QTreeWidget>
#include <QTreeWidgetItem>


DeviceSizeChart::DeviceSizeChart(QWidget *const parent) : QFrame(parent)
{
    m_layout = new QHBoxLayout();
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    m_layout->setSizeConstraint(QLayout::SetNoConstraint);
    setLayout(m_layout);

    setMinimumHeight(45);
    setMaximumHeight(45);
}

void DeviceSizeChart::setNewDevice(QTreeWidgetItem *deviceItem)
{
    if (deviceItem == nullptr)
        return;

    // Find the deviceItem at the top, just under the tree root.
    const QTreeWidget *const root = deviceItem->treeWidget();
    while ((deviceItem->parent()) && (reinterpret_cast<QTreeWidget *>(deviceItem->parent()) != root))
        deviceItem = deviceItem->parent();

    for (int i = m_layout->count() - 1; i >= 0; --i) // delete all the children
        m_layout->takeAt(i)->widget()->deleteLater();

    m_segments.clear();
    m_ratios.clear();
    m_extended_segments.clear();
    m_extended_ratios.clear();

    const StorageDevice *const device = static_cast<StorageDevice *>((deviceItem->data(1, Qt::UserRole).value<void *>()));
    QWidget *segment = nullptr;

    if (deviceItem->childCount()) {   // device with one or more partitions
        for (int i = 0; i < deviceItem->childCount(); ++i) {
            
            QTreeWidgetItem *const part_item = deviceItem->child(i);
            const StoragePartition *part = static_cast<StoragePartition *>((part_item->data(0, Qt::UserRole)).value<void *>());
            segment = new DeviceChartSeg(part_item);
            m_segments.append(segment);
            m_ratios.append(part->getSize() / static_cast<double>(device->getSize()));
            
            connect(segment, SIGNAL(deviceMenuRequested(QTreeWidgetItem *)),
                    this, SIGNAL(deviceMenuRequested(QTreeWidgetItem *)));
            
            if (part->getPedType() & PED_PARTITION_EXTENDED) {
                m_extended_layout = new QHBoxLayout();
                m_extended_layout->setSpacing(0);
                m_extended_layout->setMargin(0);
                m_extended_layout->setSizeConstraint(QLayout::SetNoConstraint);
                if (!part->isEmptyExtended()) {
                    for (int j = 0; j < part_item->childCount(); ++j) {
                        QTreeWidgetItem *const extended_item = part_item->child(j);
                        QWidget *const extended_segment = new DeviceChartSeg(extended_item);
                        part = static_cast<StoragePartition *>((extended_item->data(0, Qt::UserRole)).value<void *>());
                        m_extended_segments.append(extended_segment);
                        m_extended_ratios.append(part->getSize() / static_cast<double>(device->getSize()));
                        m_extended_layout->addWidget(extended_segment);
                        
                        connect(extended_segment, SIGNAL(deviceMenuRequested(QTreeWidgetItem *)),
                                this, SIGNAL(deviceMenuRequested(QTreeWidgetItem *)));
                    }
                }
                segment->setLayout(m_extended_layout);
            }
            m_layout->addWidget(segment);
        }
    } else {
        segment = new DeviceChartSeg(deviceItem);
        m_segments.append(segment);
        m_ratios.append(1.0);
        m_layout->addWidget(segment);

        connect(segment, SIGNAL(deviceMenuRequested(QTreeWidgetItem *)),
                this, SIGNAL(deviceMenuRequested(QTreeWidgetItem *)));
    }

    resizeSegments(width());
}

void DeviceSizeChart::resizeSegments(const int width)
{
    int max_width;
    
    for (int i = m_segments.size() - 1 ; i >= 0; --i) {
        
        max_width = (width * m_ratios[i]) - 2;
        if (max_width < 1)
            max_width = 1;

        m_segments[i]->setMaximumWidth(max_width);
    }

    for (int i = m_extended_segments.size() - 1; i >= 0; --i) {

        max_width = (width * m_extended_ratios[i]) - 2;
        if (max_width < 1)
            max_width = 1;

        m_extended_segments[i]->setMaximumWidth(max_width);
    }
}

void DeviceSizeChart::resizeEvent(QResizeEvent *const event)
{
    resizeSegments(event->size().width());
}
