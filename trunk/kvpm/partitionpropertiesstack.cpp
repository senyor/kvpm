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
#include "partitionpropertiesstack.h"
#include "partitionproperties.h"
#include "storagedevice.h"
#include "storagepartition.h"


/* This stack widget simply displays some information about the
   drive partition selected in the tree view. If nothing is selected
   an empty widget is used.
*/


PartitionPropertiesStack::PartitionPropertiesStack( QList<StorageDevice *> Devices, 
						    QWidget *parent) : QStackedWidget(parent)
{
    QList<StoragePartition *> partitions;

    QWidget *empty_widget = new QWidget(this);
    
    for(int x = 0; x < Devices.size(); x++)
	partitions << Devices[x]->getStoragePartitions() ;
    
    partition_count = partitions.size();

    for(int x = 0; x < partition_count; x++){
	partition_path_list.append( partitions[x]->getPartitionPath() );
	addWidget( new PartitionProperties( partitions[x] ) );
    }

    addWidget( empty_widget );
    setCurrentIndex( partition_count );
}

void PartitionPropertiesStack::changePartitionStackIndex(QModelIndex Index)
{
    QModelIndex model_index = Index;
    StorageDeviceItem *device_item;
    
    device_item = static_cast<StorageDeviceItem*> (model_index.internalPointer());

    QString partition_path = device_item->data(0).toString();

    setCurrentIndex( partition_count );

    for(int x = 0; x < partition_count; x++)
	if( partition_path_list[x] == partition_path )
	    setCurrentIndex(x);

}
