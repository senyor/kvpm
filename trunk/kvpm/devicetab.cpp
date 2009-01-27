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
#include <KConfigSkeleton>

#include "devicetab.h"
#include "devicetreeview.h"
#include "devicemodel.h"
#include "devicesizechart.h"
#include "deviceproperties.h"
#include "devicepropertiesstack.h"
#include "storagedevice.h"


DeviceTab::DeviceTab(QList<StorageDevice *> Devices, QWidget *parent) : 
    QWidget(parent), m_devs(Devices)
{

    QVBoxLayout *layout = new QVBoxLayout;
    QSplitter *tree_properties_splitter = new QSplitter(Qt::Horizontal);

    m_model = new StorageDeviceModel(m_devs, this);
    DeviceSizeChart *size_chart = new DeviceSizeChart(m_model, this);

    m_tree = new DeviceTreeView(this);
    m_tree->setModel(m_model);
    m_tree->expandAll();
    m_tree->setAlternatingRowColors(true); 
    m_tree->resizeColumnToContents(0);
    m_tree->resizeColumnToContents(3);
    m_tree->resizeColumnToContents(5);
    m_tree->setAllColumnsShowFocus(true);
    m_tree->setExpandsOnDoubleClick(true);
    m_tree->setSelectionBehavior(QAbstractItemView::SelectRows);

    setHiddenColumns();

    layout->addWidget(size_chart);
    layout->addWidget( tree_properties_splitter );

    tree_properties_splitter->addWidget(m_tree);
    tree_properties_splitter->addWidget( setupPropertyWidgets() );
    tree_properties_splitter->setStretchFactor( 0, 9 );
    tree_properties_splitter->setStretchFactor( 1, 2 );
    
    setLayout(layout);

    connect(m_tree, SIGNAL(clicked(const QModelIndex)), 
	    size_chart, SLOT(setNewDevice(const QModelIndex)));

    connect(m_tree, SIGNAL(activated(const QModelIndex)), 
	    size_chart, SLOT(setNewDevice(const QModelIndex)));

    // initial index setting
    m_tree->setCurrentIndex( m_model->index(0, 0) );
    size_chart->setNewDevice( m_model->index(0, 0) );
}

QScrollArea *DeviceTab::setupPropertyWidgets()
{

    QScrollArea *device_scroll = new QScrollArea();

    device_scroll->setFrameStyle(QFrame::NoFrame);

    DevicePropertiesStack *device_stack = new DevicePropertiesStack(m_devs);

    device_scroll->setBackgroundRole(QPalette::Base);
    device_scroll->setAutoFillBackground(true);
    device_scroll->setWidget(device_stack);

    connect(m_tree, SIGNAL( clicked(const QModelIndex) ), 
	    device_stack, SLOT( changeDeviceStackIndex(const QModelIndex) ));

    connect(m_tree, SIGNAL( activated(const QModelIndex) ), 
	    device_stack, SLOT( changeDeviceStackIndex(const QModelIndex) ));
    
    // initial index setting
    device_stack->changeDeviceStackIndex( m_model->index(0, 0) ); 

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
         used, 
         usage;

    skeleton.setCurrentGroup("DeviceTreeColumns");
    skeleton.addItemBool( "device",    device );
    skeleton.addItemBool( "partition", partition );
    skeleton.addItemBool( "capacity",  capacity );
    skeleton.addItemBool( "used",      used );
    skeleton.addItemBool( "usage",     usage );

    if( !device )
      m_tree->setColumnHidden( 0, true );
    if( !partition )
      m_tree->setColumnHidden( 1, true );
    if( !capacity )
      m_tree->setColumnHidden( 2, true );
    if( !used )
      m_tree->setColumnHidden( 3, true );
    if ( !usage )
      m_tree->setColumnHidden( 4, true );

}
