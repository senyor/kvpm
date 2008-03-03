/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <QtGui>

#include "devicetab.h"
#include "devicetreeview.h"
#include "devicemodel.h"
#include "devicesizechart.h"
#include "deviceproperties.h"
#include "devicepropertiesstack.h"
#include "partitionproperties.h"
#include "partitionpropertiesstack.h"
#include "storagedevice.h"


DeviceTab::DeviceTab(QList<StorageDevice *> Devices, QWidget *parent) : 
    QWidget(parent), devs(Devices)
{

    QVBoxLayout *layout;

    layout = new QVBoxLayout;
    StorageDeviceModel *model = new StorageDeviceModel(devs, this);
    DeviceSizeChart *size_chart = new DeviceSizeChart(model, this);

    tree = new DeviceTreeView(this);
    tree->setModel(model);
    tree->expandAll();
    tree->resizeColumnToContents(0);
    tree->resizeColumnToContents(3);
    layout->addWidget(size_chart);
    layout->addWidget(tree);
    layout->addWidget( setupPropertyWidgets() );
    
    setLayout(layout);

    connect(tree, SIGNAL(clicked(const QModelIndex)), 
	    size_chart, SLOT(setNewDevice(const QModelIndex)));

    connect(tree, SIGNAL(activated(const QModelIndex)), 
	    size_chart, SLOT(setNewDevice(const QModelIndex)));
}

QSplitter *DeviceTab::setupPropertyWidgets()
{
    QWidget *device_box    = new QWidget();
    QWidget *partition_box = new QWidget();
    QVBoxLayout *device_box_layout    = new QVBoxLayout();
    QVBoxLayout *partition_box_layout = new QVBoxLayout();
    device_box_layout->setMargin(0);
    partition_box_layout->setMargin(0);
    
    device_box->setLayout(device_box_layout);
    partition_box->setLayout(partition_box_layout);

    QLabel *device_label    = new QLabel("Device Properties");
    QLabel *partition_label = new QLabel("Partition / Volume Properties");
    device_label->setAlignment(Qt::AlignCenter);
    partition_label->setAlignment(Qt::AlignCenter);
    device_box_layout->addWidget(device_label);
    partition_box_layout->addWidget(partition_label);

    QScrollArea *device_scroll    = new QScrollArea();
    QScrollArea *partition_scroll = new QScrollArea();
    DevicePropertiesStack *device_stack       = new DevicePropertiesStack(devs);
    PartitionPropertiesStack *partition_stack = new PartitionPropertiesStack(devs);
    device_scroll->setBackgroundRole(QPalette::Base);
    partition_scroll->setBackgroundRole(QPalette::Base);
    device_scroll->setAutoFillBackground(true);
    partition_scroll->setAutoFillBackground(true);
    device_scroll->setWidget(device_stack);
    partition_scroll->setWidget(partition_stack);

    device_box_layout->addWidget(device_scroll);
    partition_box_layout->addWidget(partition_scroll);

    QSplitter *splitter = new QSplitter( Qt::Horizontal );
    splitter->addWidget(device_box);
    splitter->addWidget(partition_box);


    connect(tree, SIGNAL( clicked(const QModelIndex) ), 
	    device_stack, SLOT( changeDeviceStackIndex(const QModelIndex) ));

    connect(tree, SIGNAL( activated(const QModelIndex) ), 
	    device_stack, SLOT( changeDeviceStackIndex(const QModelIndex) ));

    connect(tree, SIGNAL( clicked(const QModelIndex) ), 
	    partition_stack, SLOT( changePartitionStackIndex(const QModelIndex) ));

    connect(tree, SIGNAL( activated(const QModelIndex) ), 
	    partition_stack, SLOT( changePartitionStackIndex(const QModelIndex) ));

    return splitter;
}


