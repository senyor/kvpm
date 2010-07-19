/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010 Benjamin Scott   <benscott@nwlink.com>
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


DeviceTab::DeviceTab(QWidget *parent) : QWidget(parent)
{
    m_layout = new QVBoxLayout;
    m_tree_properties_splitter = NULL;
    m_model = NULL;
    m_size_chart = NULL;
    m_tree = NULL;
    setLayout(m_layout);
}

void DeviceTab::rescan( QList<StorageDevice *> Devices )
{
    m_devs = Devices;

    if(m_model)
        m_model->deleteLater();
    m_model = new StorageDeviceModel(m_devs, this);

    if(m_size_chart)
        m_size_chart->deleteLater();
    m_size_chart = new DeviceSizeChart(m_model, this);

    if(m_tree)
        m_tree->deleteLater();
    m_tree = new DeviceTreeView(this);

    if(m_tree_properties_splitter)
        m_tree_properties_splitter->deleteLater();
    m_tree_properties_splitter = new QSplitter(Qt::Horizontal);

    m_tree->setModel(m_model);
    m_tree->expandAll();
    m_tree->setAlternatingRowColors(true); 
    m_tree->resizeColumnToContents(0);
    m_tree->resizeColumnToContents(3);
    m_tree->resizeColumnToContents(5);
    m_tree->setAllColumnsShowFocus(true);
    m_tree->setExpandsOnDoubleClick(true);
    m_tree->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_layout->addWidget( m_size_chart );
    m_layout->addWidget(m_tree_properties_splitter);

    m_tree_properties_splitter->addWidget(m_tree);
    m_tree_properties_splitter->addWidget( setupPropertyWidgets() );
    m_tree_properties_splitter->setStretchFactor( 0, 9 );
    m_tree_properties_splitter->setStretchFactor( 1, 2 );
    
    connect(m_tree, SIGNAL(clicked(const QModelIndex)), 
	    m_size_chart, SLOT(setNewDevice(const QModelIndex)));

    connect(m_tree, SIGNAL(activated(const QModelIndex)), 
	    m_size_chart, SLOT(setNewDevice(const QModelIndex)));

    // initial index setting
    m_tree->setCurrentIndex( m_model->index(0, 0) );
    m_size_chart->setNewDevice( m_model->index(0, 0) );

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
         usage,
         group,
         flags,
         mount;

    skeleton.setCurrentGroup("DeviceTreeColumns");
    skeleton.addItemBool( "device",    device );
    skeleton.addItemBool( "partition", partition );
    skeleton.addItemBool( "capacity",  capacity );
    skeleton.addItemBool( "used",      used );
    skeleton.addItemBool( "usage",     usage );
    skeleton.addItemBool( "group",     group );
    skeleton.addItemBool( "flags",     flags );
    skeleton.addItemBool( "mount",     mount );

    m_tree->setColumnHidden( 0, !device );
    m_tree->setColumnHidden( 1, !partition );
    m_tree->setColumnHidden( 2, !capacity );
    m_tree->setColumnHidden( 3, !used );
    m_tree->setColumnHidden( 4, !usage );
    m_tree->setColumnHidden( 5, !group );
    m_tree->setColumnHidden( 6, !flags );
    m_tree->setColumnHidden( 7, !mount );

}
