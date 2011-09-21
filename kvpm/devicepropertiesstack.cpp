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


#include <QtGui>

#include "storageitem.h"
#include "devicepropertiesstack.h"
#include "deviceproperties.h"
#include "storagedevice.h"
#include "storagepartition.h"


/* This stack widget simply displays some information about the
   drive or device selected in the tree view. If nothing is selected
   an empty widget is used.  */


DevicePropertiesStack::DevicePropertiesStack( QList<StorageDevice *> Devices, QWidget *parent) : 
    QStackedWidget(parent)
{
    int device_count = Devices.size();

    QList<StoragePartition *> partitions;

    for(int x = 0; x < device_count; x++){
	m_device_path_list.append( Devices[x]->getName() );
	addWidget( new DeviceProperties( Devices[x] ) );

	partitions << Devices[x]->getStoragePartitions() ;

	for(int n = 0; n < partitions.size(); n++ ){
	    m_device_path_list.append( partitions[n]->getName() );
	    addWidget( new DeviceProperties( partitions[n] ) );
	}
        partitions.clear();
    }

    addWidget( new QWidget(this) ); // empty default widget
    setCurrentIndex(0);
}

void DevicePropertiesStack::changeDeviceStackIndex(QModelIndex Index)
{
    QModelIndex model_index = Index;
    StorageItem *device_item;

    device_item = static_cast<StorageItem*> (model_index.internalPointer());

    const QString device_path = device_item->data(0).toString();
    const int list_size = m_device_path_list.size();
    setCurrentIndex(0);
    
    for(int x = 0; x < list_size; x++){
	if( m_device_path_list[x] == device_path )
	    setCurrentIndex (x);
    }
}
