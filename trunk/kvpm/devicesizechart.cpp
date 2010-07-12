/*
 *
 * 
 * Copyright (C) 2008, 2009 Benjamin Scott   <benscott@nwlink.com>
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
    device_model = model;
    QModelIndex index;

    setFrameStyle( QFrame::Sunken | QFrame::Panel );
    setLineWidth(2);

    index = model->index(0,0);
    layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->setSizeConstraint(QLayout::SetNoConstraint);
    setLayout(layout);

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

    for(int x = layout->count() - 1; x >= 0; x--){  // delete all the children
	QWidget *old_widget = (layout->takeAt(x))->widget();
	old_widget->setParent(0);
	delete old_widget;
    }

    segments.clear();
    ratios.clear();
    extended_segments.clear();
    extended_ratios.clear();
    while(index.parent() != QModelIndex())
	index = index.parent();

    device_item = static_cast<StorageDeviceItem*> (index.internalPointer());

    if( !device_item->childCount() ){
	QWidget *empty_widget = new QWidget(this);
	layout->addWidget(empty_widget);
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
	segments.append(segment);
	ratios.append(ratio);
	if( part_type & 0x02 ){  // extended partition
	    extended_layout = new QHBoxLayout();
	    extended_layout->setSpacing(0);
	    extended_layout->setMargin(0);
	    extended_layout->setSizeConstraint(QLayout::SetNoConstraint);
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
		extended_segments.append(extended_segment);
		extended_ratios.append(ratio);

		extended_layout->addWidget(extended_segment);
	    }
	    segment->setLayout(extended_layout);
	}
	layout->addWidget(segment);
    }

    for(int x = segments.size() - 1 ; x >= 0; x--){

	max_segment_width = (int)( (width()  * ratios[x]) - 2 );
	if( max_segment_width < 1 )
	    max_segment_width = 1;
	
	segments[x]->setMaximumWidth(max_segment_width);
    }
    
    for(int x = extended_segments.size() - 1; x >= 0; x--){
	
	max_segment_width = (int)( (width()  * extended_ratios[x]) - 2 );
	if( max_segment_width < 1 )
	    max_segment_width = 1;
	
	extended_segments[x]->setMaximumWidth(max_segment_width);
    }
}

void DeviceSizeChart::resizeEvent(QResizeEvent *event)
{
    int max_segment_width;
    int new_width = (event->size()).width();

    for(int x = segments.size() - 1 ; x >= 0; x--){

	max_segment_width = (int)( (new_width * ratios[x]) - 2 );
	if( max_segment_width < 1 )
	    max_segment_width = 1;
	
	segments[x]->setMaximumWidth(max_segment_width);
    }
    
    for(int x = extended_segments.size() - 1; x >= 0; x--){
	
	max_segment_width = (int)( (new_width  * extended_ratios[x]) - 2 );
	if( max_segment_width < 1 )
	    max_segment_width = 1;
	
	extended_segments[x]->setMaximumWidth(max_segment_width);
    }
}
