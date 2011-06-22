/*
 *
 * 
 * Copyright (C) 2008, 2009, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include <KSeparator>
#include <QtGui>

#include "devicesizechart.h"
#include "storagedevice.h"
#include "storagepartition.h"

DeviceSizeChart::DeviceSizeChart(StorageDeviceModel *model, QWidget *parent) : QFrame(parent)
{
    m_device_model = model;
    QModelIndex index;

    setFrameStyle( QFrame::Sunken | QFrame::Panel );
    setLineWidth(2);

    index = model->index(0,0);
    m_layout = new QHBoxLayout();
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    m_layout->setSizeConstraint(QLayout::SetNoConstraint);
    setLayout(m_layout);

    setNewDevice(index);
    setMinimumHeight(45);
    setMaximumHeight(45);
}

void DeviceSizeChart::setNewDevice(QModelIndex index)
{
    QWidget *segment, *extended_segment;
    QString usage, path;
    double ratio;
    StorageDeviceItem *device_item, *partition_item, *extended_item;
    StoragePartition *partition;
    StorageDevice    *device;
    long long part_size, device_size;
    int max_segment_width;
    unsigned int part_type;

    for(int x = m_layout->count() - 1; x >= 0; x--){  // delete all the children
	QWidget *old_widget = (m_layout->takeAt(x))->widget();
	old_widget->setParent(0);
	delete old_widget;
    }

    m_segments.clear();
    m_ratios.clear();
    m_extended_segments.clear();
    m_extended_ratios.clear();
    while(index.parent() != QModelIndex())
	index = index.parent();

    device_item = static_cast<StorageDeviceItem*> (index.internalPointer());

    if( !device_item->childCount() ){
	device    = (StorageDevice *) (( device_item->dataAlternate(1)).value<void *>() );
        if( !device->isPhysicalVolume() )
            usage = "physical volume";
        else
            usage = "";
        segment = new DeviceChartSeg(device_item);	
        m_segments.append(segment);
        m_ratios.append(1.0);
        m_layout->addWidget(segment);
    }

    for(int x = 0; x < device_item->childCount(); x++){
	partition_item = device_item->child(x);

        partition = (StoragePartition *) (( partition_item->dataAlternate(0)).value<void *>() );
	device    = (StorageDevice *) (( partition_item->dataAlternate(1)).value<void *>() );

	part_type = partition->getPedType();
	part_size = partition->getSize();
	device_size = device->getSize();

	if( partition->isPV() )
	    usage = "physical volume";
	else
	    usage = (partition_item->data(3)).toString();

	ratio = part_size / (double) device_size;
	segment = new DeviceChartSeg(partition_item);	
	m_segments.append(segment);
	m_ratios.append(ratio);
	if( part_type & 0x02 ){  // extended partition
	    m_extended_layout = new QHBoxLayout();
	    m_extended_layout->setSpacing(0);
	    m_extended_layout->setMargin(0);
	    m_extended_layout->setSizeConstraint(QLayout::SetNoConstraint);
            if( ! partition->isEmpty() ){
                for(int y = 0 ; y < partition_item->childCount(); y++){
                    extended_item = partition_item->child(y);
                    partition = (StoragePartition *) (( extended_item->dataAlternate(0)).value<void *>() );
                    device = (StorageDevice *) (( extended_item->dataAlternate(1)).value<void *>() );
                    part_type = partition->getPedType();
                    part_size = partition->getSize();
                    device_size = device->getSize();
                    if( partition->isPV() )
                        usage = "physical volume";
                    else
                        usage = (extended_item->data(3)).toString();
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

    for(int x = m_segments.size() - 1 ; x >= 0; x--){

	max_segment_width = (int)( ( width() * m_ratios[x]) - 2 );
	if( max_segment_width < 1 )
	    max_segment_width = 1;
	
	m_segments[x]->setMaximumWidth(max_segment_width);
    }
    
    for(int x = m_extended_segments.size() - 1; x >= 0; x--){
	
	max_segment_width = (int)( ( width() * m_extended_ratios[x] ) - 2 );
	if( max_segment_width < 1 )
	    max_segment_width = 1;
	
	m_extended_segments[x]->setMaximumWidth(max_segment_width);
    }
}

void DeviceSizeChart::resizeEvent(QResizeEvent *event)
{
    int max_segment_width;
    int new_width = (event->size()).width();

    for(int x = m_segments.size() - 1 ; x >= 0; x--){

	max_segment_width = (int)( (new_width * m_ratios[x]) - 2 );
	if( max_segment_width < 1 )
	    max_segment_width = 1;
	
	m_segments[x]->setMaximumWidth(max_segment_width);
    }
    
    for(int x = m_extended_segments.size() - 1; x >= 0; x--){
	
	max_segment_width = (int)( (new_width  * m_extended_ratios[x]) - 2 );
	if( max_segment_width < 1 )
	    max_segment_width = 1;
	
	m_extended_segments[x]->setMaximumWidth(max_segment_width);
    }
}
