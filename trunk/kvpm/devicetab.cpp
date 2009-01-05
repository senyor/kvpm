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


#include <QtGui>
#include <KLocale>

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
    QWidget(parent), m_devs(Devices)
{

    QVBoxLayout *layout;

    layout = new QVBoxLayout;
    m_model = new StorageDeviceModel(m_devs, this);
    DeviceSizeChart *size_chart = new DeviceSizeChart(m_model, this);

    m_tree = new DeviceTreeView(this);
    m_tree->setModel(m_model);
    m_tree->expandAll();
    m_tree->setAlternatingRowColors(true); 
    m_tree->resizeColumnToContents(0);
    m_tree->resizeColumnToContents(3);
    m_tree->setAllColumnsShowFocus(true);
    m_tree->setExpandsOnDoubleClick(true);
    m_tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(size_chart);
    layout->addWidget(m_tree);
    layout->addWidget( setupPropertyWidgets() );
    
    setLayout(layout);


    connect(m_tree, SIGNAL(clicked(const QModelIndex)), 
	    size_chart, SLOT(setNewDevice(const QModelIndex)));

    connect(m_tree, SIGNAL(activated(const QModelIndex)), 
	    size_chart, SLOT(setNewDevice(const QModelIndex)));

    // initial index setting
    m_tree->setCurrentIndex( m_model->index(0, 0) );
    size_chart->setNewDevice( m_model->index(0, 0) );
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

    QLabel *device_label    = new QLabel( i18n("Device Properties") );
    QLabel *partition_label = new QLabel( i18n("Partition / Volume Properties") );
    device_label->setAlignment(Qt::AlignCenter);
    partition_label->setAlignment(Qt::AlignCenter);
    device_box_layout->addWidget(device_label);
    partition_box_layout->addWidget(partition_label);

    QScrollArea *device_scroll    = new QScrollArea();
    QScrollArea *partition_scroll = new QScrollArea();
    DevicePropertiesStack *device_stack       = new DevicePropertiesStack(m_devs);
    PartitionPropertiesStack *partition_stack = new PartitionPropertiesStack(m_devs);
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


    connect(m_tree, SIGNAL( clicked(const QModelIndex) ), 
	    device_stack, SLOT( changeDeviceStackIndex(const QModelIndex) ));

    connect(m_tree, SIGNAL( activated(const QModelIndex) ), 
	    device_stack, SLOT( changeDeviceStackIndex(const QModelIndex) ));

    connect(m_tree, SIGNAL( clicked(const QModelIndex) ), 
	    partition_stack, SLOT( changePartitionStackIndex(const QModelIndex) ));

    connect(m_tree, SIGNAL( activated(const QModelIndex) ), 
	    partition_stack, SLOT( changePartitionStackIndex(const QModelIndex) ));

    // initial index setting
    partition_stack->changePartitionStackIndex( m_model->index(0, 0) );
    device_stack->changeDeviceStackIndex( m_model->index(0, 0) ); 

    return splitter;
}


