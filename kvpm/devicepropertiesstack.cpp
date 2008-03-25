/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Klvm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <QtGui>

#include "devicemodel.h"
#include "devicepropertiesstack.h"
#include "deviceproperties.h"
#include "storagedevice.h"

/* This stack widget simply displays some information about the
   drive or device selected in the tree view. If nothing is selected
   an empty widget is used.
*/

DevicePropertiesStack::DevicePropertiesStack( QList<StorageDevice *> Devices, QWidget *parent) : 
    QStackedWidget(parent)
{

    device_count = Devices.size();

    QWidget *empty_widget = new QWidget(this);
    
    for(int x = 0; x < device_count; x++){
	device_path_list.append( Devices[x]->getDevicePath() );
	addWidget( new DeviceProperties( Devices[x] ) );
    }

    addWidget( empty_widget );
    setCurrentIndex( device_count );
}

void DevicePropertiesStack::changeDeviceStackIndex(QModelIndex Index)
{
    QModelIndex model_index = Index;
    StorageDeviceItem *device_item;

    while(model_index.parent() != QModelIndex())
        model_index = model_index.parent();

    device_item = static_cast<StorageDeviceItem*> (model_index.internalPointer());

    QString device_path = device_item->data(0).toString();

    setCurrentIndex( device_count );
    
    for(int x = 0; x < device_count; x++)
	if( device_path_list[x] == device_path )
	    setCurrentIndex ( x );
}
