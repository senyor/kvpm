/*
 *
 *
 * Copyright (C) 2008, 2009, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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

#include <KSeparator>

#include <QHBoxLayout>
#include <QResizeEvent>
#include <QTreeWidget>
#include <QTreeWidgetItem>


DeviceSizeChart::DeviceSizeChart(QWidget *parent) : QFrame(parent)
{
    //    setFrameStyle(QFrame::Sunken | QFrame::Panel);
    //    setLineWidth(2);

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
    QWidget *segment, *extended_segment;
    QString usage, path;
    double ratio;
    QTreeWidgetItem  *partition_item, *extended_item;
    StoragePartition *partition;
    StorageDevice    *device;
    long long part_size, device_size;
    int max_segment_width;
    unsigned int part_type;

    if (deviceItem == NULL)
        return;

    while ((deviceItem->parent() != NULL) && ((QTreeWidget *)deviceItem->parent() != m_tree))
        deviceItem = deviceItem->parent();

    for (int x = m_layout->count() - 1; x >= 0; x--) // delete all the children
        m_layout->takeAt(x)->widget()->deleteLater();

    m_segments.clear();
    m_ratios.clear();
    m_extended_segments.clear();
    m_extended_ratios.clear();
    device = (StorageDevice *)((deviceItem->data(1, Qt::UserRole).value<void *>()));

    if (!deviceItem->childCount()) {

        if (!device->isPhysicalVolume())
            usage = "physical volume";
        else
            usage = "";

        segment = new DeviceChartSeg(deviceItem);
        m_segments.append(segment);
        m_ratios.append(1.0);
        m_layout->addWidget(segment);
    }

    for (int x = 0; x < deviceItem->childCount(); x++) {
        partition_item = deviceItem->child(x);

        partition = (StoragePartition *)((partition_item->data(0, Qt::UserRole)).value<void *>());

        part_type = partition->getPedType();
        part_size = partition->getSize();
        device_size = device->getSize();

        if (partition->isPhysicalVolume())
            usage = "physical volume";
        else
            usage = (partition_item->data(3, Qt::DisplayRole)).toString();

        ratio = part_size / (double) device_size;
        segment = new DeviceChartSeg(partition_item);
        m_segments.append(segment);
        m_ratios.append(ratio);
        if (part_type & 0x02) {  // extended partition
            m_extended_layout = new QHBoxLayout();
            m_extended_layout->setSpacing(0);
            m_extended_layout->setMargin(0);
            m_extended_layout->setSizeConstraint(QLayout::SetNoConstraint);
            if (!partition->isEmptyExtended()) {
                for (int y = 0 ; y < partition_item->childCount(); y++) {
                    extended_item = partition_item->child(y);
                    partition = (StoragePartition *)((extended_item->data(0, Qt::UserRole)).value<void *>());
                    device = (StorageDevice *)((extended_item->data(1, Qt::UserRole)).value<void *>());
                    part_type = partition->getPedType();
                    part_size = partition->getSize();
                    device_size = device->getSize();
                    if (partition->isPhysicalVolume())
                        usage = "physical volume";
                    else
                        usage = (extended_item->data(3, Qt::DisplayRole)).toString();
                    extended_segment = new DeviceChartSeg(extended_item);
                    ratio = part_size / (double) device_size;
                    m_extended_segments.append(extended_segment);
                    m_extended_ratios.append(ratio);

                    m_extended_layout->addWidget(extended_segment);
                }
            }
            segment->setLayout(m_extended_layout);
        }
        m_layout->addWidget(segment);
    }

    for (int x = m_segments.size() - 1 ; x >= 0; x--) {

        max_segment_width = (int)((width() * m_ratios[x]) - 2);
        if (max_segment_width < 1)
            max_segment_width = 1;

        m_segments[x]->setMaximumWidth(max_segment_width);
    }

    for (int x = m_extended_segments.size() - 1; x >= 0; x--) {

        max_segment_width = (int)((width() * m_extended_ratios[x]) - 2);
        if (max_segment_width < 1)
            max_segment_width = 1;

        m_extended_segments[x]->setMaximumWidth(max_segment_width);
    }
}

void DeviceSizeChart::resizeEvent(QResizeEvent *event)
{
    int max_segment_width;
    int new_width = (event->size()).width();

    for (int x = m_segments.size() - 1 ; x >= 0; x--) {

        max_segment_width = (int)((new_width * m_ratios[x]) - 2);
        if (max_segment_width < 1)
            max_segment_width = 1;

        m_segments[x]->setMaximumWidth(max_segment_width);
    }

    for (int x = m_extended_segments.size() - 1; x >= 0; x--) {

        max_segment_width = (int)((new_width  * m_extended_ratios[x]) - 2);
        if (max_segment_width < 1)
            max_segment_width = 1;

        m_extended_segments[x]->setMaximumWidth(max_segment_width);
    }
}
