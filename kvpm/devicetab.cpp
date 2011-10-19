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
    m_layout = new QVBoxLayout;
    m_tree_properties_splitter = NULL;

    m_size_chart = NULL;
    m_tree = NULL;
    setLayout(m_layout);
}

void DeviceTab::rescan( QList<StorageDevice *> Devices )
{
    m_devs = Devices;

    if(m_tree)
        m_tree->deleteLater();
    m_tree = new DeviceTree(m_devs, this);

    if(m_size_chart)
        m_size_chart->deleteLater();
    m_size_chart = new DeviceSizeChart(m_tree, this);

    if(m_tree_properties_splitter)
        m_tree_properties_splitter->deleteLater();
    m_tree_properties_splitter = new QSplitter(Qt::Horizontal);


    m_layout->addWidget( m_size_chart );
    m_layout->addWidget(m_tree_properties_splitter);

    m_tree_properties_splitter->addWidget(m_tree);
    m_tree_properties_splitter->addWidget( setupPropertyWidgets() );
    m_tree_properties_splitter->setStretchFactor( 0, 9 );
    m_tree_properties_splitter->setStretchFactor( 1, 2 );
    
    connect(m_tree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), 
	    m_size_chart, SLOT(setNewDevice(QTreeWidgetItem*)));

    connect(m_tree, SIGNAL(itemActivated(QTreeWidgetItem*, int)), 
	    m_size_chart, SLOT(setNewDevice(QTreeWidgetItem*)));

    setHiddenColumns();

    return;
}

QScrollArea *DeviceTab::setupPropertyWidgets()
{
    QScrollArea *device_scroll = new QScrollArea();

    device_scroll->setFrameStyle(QFrame::NoFrame);

    DevicePropertiesStack *device_stack = new DevicePropertiesStack(m_devs);

    device_scroll->setBackgroundRole(QPalette::Base);
    device_scroll->setAutoFillBackground(true);
    device_scroll->setWidget(device_stack);

    connect(m_tree, SIGNAL( itemClicked(QTreeWidgetItem*, int) ), 
	    device_stack, SLOT( changeDeviceStackIndex(QTreeWidgetItem*) ));

    connect(m_tree, SIGNAL( itemActivated(QTreeWidgetItem*, int) ), 
	    device_stack, SLOT( changeDeviceStackIndex(QTreeWidgetItem*) ));

    if( m_tree->currentItem() )
        device_stack->changeDeviceStackIndex( m_tree->currentItem() );

    device_scroll->setWidgetResizable(true);
    device_scroll->setBackgroundRole(QPalette::Base);
    device_scroll->setAutoFillBackground(true);

    return device_scroll;
}

void DeviceTab::setHiddenColumns()
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

    skeleton.setCurrentGroup("DeviceTreeColumns");
    skeleton.addItemBool( "device",       device );
    skeleton.addItemBool( "partition",    partition );
    skeleton.addItemBool( "capacity",     capacity );
    skeleton.addItemBool( "remaining", remaining );
    skeleton.addItemBool( "usage",        usage );
    skeleton.addItemBool( "group",        group );
    skeleton.addItemBool( "flags",        flags );
    skeleton.addItemBool( "mount",        mount );

    m_tree->setColumnHidden( 0, !device );
    m_tree->setColumnHidden( 1, !partition );
    m_tree->setColumnHidden( 2, !capacity );
    m_tree->setColumnHidden( 3, !remaining );
    m_tree->setColumnHidden( 4, !usage );
    m_tree->setColumnHidden( 5, !group );
    m_tree->setColumnHidden( 6, !flags );
    m_tree->setColumnHidden( 7, !mount );
}
