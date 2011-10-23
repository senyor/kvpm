/*
 *
 * 
 * Copyright (C) 2008, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "devicepropertiesstack.h"

#include <QtGui>

#include "deviceproperties.h"
#include "devicetree.h"
#include "storagedevice.h"
#include "storagepartition.h"


/* This stack widget simply displays some information about the
   drive or device selected in the tree view. If nothing is selected
   an empty widget is used.  */


DevicePropertiesStack::DevicePropertiesStack(QWidget *parent) : QStackedWidget(parent)
{
    addWidget( getDefaultWidget() );
    setCurrentIndex(0);
}

void DevicePropertiesStack::changeDeviceStackIndex(QTreeWidgetItem *item)
{
    setCurrentIndex(0);

    if( !item )
        return;

    const QString device_path = item->data(0, Qt::DisplayRole).toString();
    const int list_size = m_device_path_list.size();
    
    for(int x = 0; x < list_size; x++){
	if( m_device_path_list[x] == device_path )
	    setCurrentIndex (x);
    }
}

void DevicePropertiesStack::loadData( QList<StorageDevice *> devices)
{
    QWidget *stackWidget;
    QList<StoragePartition *> partitions;

    m_device_path_list.clear();

    for(int x = count() - 1; x >= 0; x--){ // delete old member widgets
        stackWidget = widget(x);
        removeWidget(stackWidget);
        delete stackWidget;
    }

    for(int x = devices.size() - 1; x >= 0; x--){
	m_device_path_list.append( devices[x]->getName() );
	addWidget( new DeviceProperties( devices[x] ) );
	partitions << devices[x]->getStoragePartitions() ;

	for(int n = 0; n < partitions.size(); n++ ){
	    m_device_path_list.append( partitions[n]->getName() );
	    addWidget( new DeviceProperties( partitions[n] ) );
	}

        partitions.clear();
    }

    addWidget( getDefaultWidget() );
    setCurrentIndex(0);
}

QWidget *DevicePropertiesStack::getDefaultWidget()
{
    QWidget *dw = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout();

    layout->addWidget( new QLabel("Random String For The Layout") );
    dw->setLayout(layout);
    return dw;
}
