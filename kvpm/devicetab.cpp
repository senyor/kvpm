/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "devicetab.h"

#include <QtGui>
#include <KLocale>
#include <KConfigSkeleton>

#include "devicetree.h"
#include "devicesizechart.h"
#include "deviceproperties.h"
#include "devicepropertiesstack.h"
#include "storagedevice.h"


DeviceTab::DeviceTab(QWidget *parent) : QWidget(parent)
{
    m_tree = new DeviceTree(this);
    m_size_chart = new DeviceSizeChart(m_tree, this);

    connect(m_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem *)), 
            m_size_chart, SLOT(setNewDevice(QTreeWidgetItem*)));

    m_tree_properties_splitter = new QSplitter(Qt::Horizontal);

    m_layout = new QVBoxLayout;
    m_layout->addWidget(m_size_chart);
    m_layout->addWidget(m_tree_properties_splitter);

    m_tree_properties_splitter->addWidget(m_tree);
    m_tree_properties_splitter->addWidget( setupPropertyStack() );
    m_tree_properties_splitter->setStretchFactor( 0, 9 );
    m_tree_properties_splitter->setStretchFactor( 1, 2 );

    setLayout(m_layout);
}

void DeviceTab::rescan( QList<StorageDevice *> devices )
{
    m_device_stack->loadData(devices);
    m_tree->loadData(devices);
}

QScrollArea *DeviceTab::setupPropertyStack()
{
    m_device_stack = new DevicePropertiesStack();

    QScrollArea *device_scroll = new QScrollArea();
    device_scroll->setFrameStyle(QFrame::NoFrame);
    device_scroll->setBackgroundRole(QPalette::Base);
    device_scroll->setAutoFillBackground(true);
    device_scroll->setWidget(m_device_stack);
    device_scroll->setWidgetResizable(true);
    device_scroll->setBackgroundRole(QPalette::Base);
    device_scroll->setAutoFillBackground(true);

    connect(m_tree, SIGNAL( currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*) ), 
	    m_device_stack, SLOT( changeDeviceStackIndex(QTreeWidgetItem*) ));

    if( m_tree->currentItem() )
        m_device_stack->changeDeviceStackIndex( m_tree->currentItem() );

    return device_scroll;
}
