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


#include <QtGui>
#include "devicemodel.h"
#include "physvol.h"
#include "volgroup.h"
#include "storagedevice.h"
#include "storagepartition.h"


extern QString sizeToString(long long bytes);


StorageDeviceItem::StorageDeviceItem(const QList<QVariant> &data, 
				     const QList<QVariant> &dataAlternate, 
				     StorageDeviceItem *parent)
{
    parentItem = parent;
    itemData = data;
    itemDataAlternate = dataAlternate;
}

StorageDeviceItem::~StorageDeviceItem()
{
    qDeleteAll(childItems);
}

void StorageDeviceItem::appendChild(StorageDeviceItem *item)
{
    childItems.append(item);
}

StorageDeviceItem *StorageDeviceItem::child(int row)
{
    return childItems[row];
}

int StorageDeviceItem::childCount() const
{
    return childItems.count();
}

int StorageDeviceItem::row() const
{
    if (parentItem)
	return parentItem->childItems.indexOf(const_cast<StorageDeviceItem *>(this));
    
    return 0;
}

int StorageDeviceItem::columnCount() const
{
    return itemData.count();
}

QVariant StorageDeviceItem::data(int column) const
{
    return itemData.value(column);
}

QVariant StorageDeviceItem::dataAlternate(int column) const
{
    return itemDataAlternate.value(column);
}

StorageDeviceItem *StorageDeviceItem::parent() {
    return parentItem;
}


StorageDeviceModel::StorageDeviceModel(QList<StorageDevice *> devices, QObject *parent) : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << "Device" << "Type" << "Capacity" << "Remaining" << "Usage" 
	     << "Group" << "Flags" << "Mount point" ;
    
    rootItem = new StorageDeviceItem(rootData, rootData);
    setupModelData(devices, rootItem);

    reset();
}

StorageDeviceModel::~StorageDeviceModel()
{
    delete rootItem;
}


QModelIndex StorageDeviceModel::index(int row, int column, const QModelIndex &parent)
    const
{
    StorageDeviceItem *parentItem;

    if (!parent.isValid())
	parentItem = rootItem;
    else
	parentItem = static_cast<StorageDeviceItem*>(parent.internalPointer());

    StorageDeviceItem *childItem = parentItem->child(row);
    if (childItem)
	return createIndex(row, column, childItem);
    else
	return QModelIndex();
 }

QModelIndex StorageDeviceModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
	return QModelIndex();
    
    StorageDeviceItem *childItem = static_cast<StorageDeviceItem*>(index.internalPointer());
    StorageDeviceItem *parentItem = childItem->parent();
    
    if (parentItem == rootItem)
	return QModelIndex();
    
    return createIndex(parentItem->row(), 0, parentItem);
}

int StorageDeviceModel::rowCount(const QModelIndex &parent) const
{
    StorageDeviceItem *parentItem;
    
    if (!parent.isValid())
	parentItem = rootItem;
    else
	parentItem = static_cast<StorageDeviceItem*>(parent.internalPointer());
    
    return parentItem->childCount();
}

int StorageDeviceModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
	return static_cast<StorageDeviceItem*>(parent.internalPointer())->columnCount();
    else
	return rootItem->columnCount();
}

QVariant StorageDeviceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
	return QVariant();
    
    if ((role != Qt::DisplayRole) && (role != Qt::UserRole))
	return QVariant();

    StorageDeviceItem *item = static_cast<StorageDeviceItem*>(index.internalPointer());

    if (role == Qt::UserRole)
	return item->dataAlternate(index.column());


    return item->data(index.column());
}

Qt::ItemFlags StorageDeviceModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
	return Qt::ItemIsEnabled;
    
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant StorageDeviceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	return rootItem->data(section);

    return QVariant();
}

void StorageDeviceModel::setupModelData(QList<StorageDevice *> devices, StorageDeviceItem *parent)
{
    StorageDeviceItem *child, *extended = 0;
    StorageDevice *dev;
    QList<QVariant> data, dataAlternate;
    StoragePartition *part;
    QString type;
    QVariant part_variant;
    QVariant dev_variant;
    PhysVol *pv;
    
    /* dataAlternate
       0:  pointer to storagepartition if partition, else "" 
       1:  pointer to storagedevice
    */

    for(int x = 0; x < devices.size(); x++){
	data.clear();
	dataAlternate.clear();
	dev = devices[x];
        dev_variant.setValue( (void *) dev);
        if(dev->isPhysicalVolume()){
            pv = dev->getPhysicalVolume();
            data << dev->getDevicePath() << "" << sizeToString(dev->getSize());
            data << QString("%1 ( %%2 ) ").arg( sizeToString( pv->getUnused() )).arg( 100 - pv->getPercentUsed() );

            data  << "physical volume" << pv->getVolGroup()->getName();
            dataAlternate << "" << dev_variant;
        }
        else{
            data << dev->getDevicePath() << "" << sizeToString(dev->getSize());
            dataAlternate << "" << dev_variant;
        }
	
        StorageDeviceItem *item = new StorageDeviceItem(data, dataAlternate, parent);
        parent->appendChild(item);
        for(int y = 0; y < dev->getPartitionCount(); y++){
            data.clear();
            dataAlternate.clear();
            part = dev->getStoragePartitions()[y];
            type = part->getType();
            
            data << part->getName() << type << sizeToString(part->getSize());
            
            part_variant.setValue( (void *) part);
            
            dataAlternate << part_variant << dev_variant;
            
            if(part->isPV()){
                pv = part->getPhysicalVolume();
                data << QString("%1 (%%2) ").arg( sizeToString( (pv->getUnused()))).arg(100 - pv->getPercentUsed() )
                    
                     << "physical volume"
                     << pv->getVolGroup()->getName()
                     << (part->getFlags()).join(", ") 
                     << "";
            }
            else{
                if(part->getFilesystemSize() > -1 && part->getFilesystemUsed() > -1)
                    data << QString("%1 (%%2) ").arg(sizeToString(part->getFilesystemSize() - part->getFilesystemUsed())).arg(100 - part->getPercentUsed() ); 
                else
                    data << "";
 
                data << part->getFilesystem() << "" << (part->getFlags()).join(", ");
                
                if(part->isMounted())
                    data << (part->getMountPoints())[0];
                else if( part->isBusy() && ( part->getFilesystem() == "swap" ) )
                    data << "swapping";
                else
                    data << "";
            }
            
            if(type == "extended"){
                extended = new StorageDeviceItem(data, dataAlternate, item);
                item->appendChild(extended);
            }
            else if( !( (type == "logical") || (type == "freespace (logical)") ) ){
                child = new StorageDeviceItem(data, dataAlternate, item);
                item->appendChild(child);
            }
            else if(extended){
                child = new StorageDeviceItem(data, dataAlternate, extended);
                extended->appendChild(child);
            }
            else
                qDebug() << "devicemodel.cpp How did we get here?";
        }

    }
}

