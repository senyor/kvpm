/*
 *
 * 
 * Copyright (C) 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "storagemodel.h"

#include <KIcon>
#include <QtGui>

#include "misc.h"
#include "physvol.h"
#include "storageitem.h"
#include "storagedevice.h"
#include "storagepartition.h"
#include "volgroup.h"


StorageModel::StorageModel(QList<StorageDevice *> devices, QObject *parent) : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << "Device" << "Type" << "Capacity" << "Remaining" << "Usage" 
	     << "Group" << "Flags" << "Mount point" ;
    
    rootItem = new StorageItem(rootData, rootData);
    setupModelData(devices, rootItem);

    reset();
}

StorageModel::~StorageModel()
{
    delete rootItem;
}

QModelIndex StorageModel::index(int row, int column, const QModelIndex &parent)
    const
{
    StorageItem *parentItem;

    if (!parent.isValid())
	parentItem = rootItem;
    else
	parentItem = static_cast<StorageItem*>(parent.internalPointer());

    StorageItem *childItem = parentItem->child(row);

    if (childItem)
	return createIndex(row, column, childItem);
    else
	return QModelIndex();
 }

QModelIndex StorageModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
	return QModelIndex();
    
    StorageItem *childItem = static_cast<StorageItem*>(index.internalPointer());
    StorageItem *parentItem = childItem->parent();
    
    if (parentItem == rootItem)
	return QModelIndex();
    
    return createIndex(parentItem->row(), 0, parentItem);
}

int StorageModel::rowCount(const QModelIndex &parent) const
{
    StorageItem *parentItem;
    
    if (!parent.isValid())
	parentItem = rootItem;
    else
	parentItem = static_cast<StorageItem*>(parent.internalPointer());
    
    return parentItem->childCount();
}

int StorageModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
	return static_cast<StorageItem*>(parent.internalPointer())->columnCount();
    else
	return rootItem->columnCount();
}

QVariant StorageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
	return QVariant();

    if( (role == Qt::TextAlignmentRole) && (index.column() > 0 && index.column() < 7) )
        return QVariant(Qt::AlignRight);

    StorageItem *item = static_cast<StorageItem*>(index.internalPointer());

    if ( role == Qt::UserRole )
	return item->dataAlternate(index.column());
    else if( role == Qt::DecorationRole )
        return item->dataIcon(index.column());
    else if( role == Qt::ToolTipRole )
        return item->dataToolTip(index.column());
    else if( role == Qt::DisplayRole )
        return item->data(index.column());
    else
        return QVariant();
}

Qt::ItemFlags StorageModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
	return Qt::ItemIsEnabled;
    
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant StorageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	return rootItem->data(section);

    if( role == Qt::TextAlignmentRole )
        return QVariant(Qt::AlignCenter);

    return QVariant();
}

void StorageModel::setupModelData(QList<StorageDevice *> devices, StorageItem *parent)
{
    StorageItem *child, *extended = 0;
    StorageDevice *dev;
    QList<QVariant> data, dataAlternate;
    StoragePartition *part;
    QString type;
    QVariant part_variant;
    QVariant dev_variant;
    PhysVol *pv;
    
    /* 
       dataAlternate
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
            data << dev->getName() << "" << sizeToString(dev->getSize());
            data << QString("%1 (%%2) ").arg( sizeToString( pv->getUnused() )).arg( 100 - pv->getPercentUsed() );

            data  << "physical volume" << pv->getVG()->getName();
            dataAlternate << "" << dev_variant;
        }
        else{
            data << dev->getName() << "" << sizeToString(dev->getSize());
            dataAlternate << "" << dev_variant;
        }
	
        StorageItem *item = new StorageItem(data, dataAlternate, parent);

        if( dev->isPhysicalVolume() ){
            if( dev->getPhysicalVolume()->isActive() ){
                item->setIcon(4, KIcon("lightbulb"));
                item->setToolTip(4, "Active");
            }
            else{
                item->setIcon(4, KIcon("lightbulb_off"));
                item->setToolTip(4, "Inactive");
            }
        }

        parent->appendChild(item);
        for(int y = 0; y < dev->getPartitionCount(); y++){
            data.clear();
            dataAlternate.clear();
            part = dev->getStoragePartitions()[y];
            type = part->getType();
            
            data << part->getName() << type << sizeToString(part->getSize());
            
            part_variant.setValue( (void *) part);
            
            dataAlternate << part_variant << dev_variant;
            
            if(part->isPhysicalVolume()){
                pv = part->getPhysicalVolume();
                data << QString("%1 (%%2) ").arg( sizeToString( (pv->getUnused()))).arg(100 - pv->getPercentUsed() )
                    
                     << "physical volume"
                     << pv->getVG()->getName()
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
                extended = new StorageItem(data, dataAlternate, item);
                item->appendChild(extended);
            }
            else if( !( (type == "logical") || (type == "freespace (logical)") ) ){
                child = new StorageItem(data, dataAlternate, item);

                if( part->isMountable() ){
                    if( part->isMounted() ){
                        child->setIcon(4, KIcon("emblem-mounted"));
                        child->setToolTip(4, "Mounted");
                    }
                    else{
                        child->setIcon(4, KIcon("emblem-unmounted"));
                        child->setToolTip(4, "Unmounted");
                    }
                }
                if( part->isPhysicalVolume() ){
                    if( part->getPhysicalVolume()->isActive() ){
                        child->setIcon(4, KIcon("lightbulb"));
                        child->setToolTip(4, "Active");
                    }
                    else{
                        child->setIcon(4, KIcon("lightbulb_off"));
                        child->setToolTip(4, "Inactive");
                    }
                }

                item->appendChild(child);
            }
            else if(extended){
                child = new StorageItem(data, dataAlternate, extended);

                if( part->isMountable() ){
                    if( part->isMounted() ){
                        child->setIcon(4, KIcon("emblem-mounted"));
                        child->setToolTip(4, "Mounted");
                            }
                    else{
                        child->setIcon(4, KIcon("emblem-unmounted"));
                        child->setToolTip(4, "Unmounted");
                    }
                }

                if( part->isPhysicalVolume() ){
                    if( part->getPhysicalVolume()->isActive() ){
                        child->setIcon(4, KIcon("lightbulb"));
                        child->setToolTip(4, "Active");
                    }
                    else{
                        child->setIcon(4, KIcon("lightbulb_off"));
                        child->setToolTip(4, "Inactive");
                    }
                }

                extended->appendChild(child);
            }
            else
                qDebug() << "devicemodel.cpp How did we get here?";
        }
    }
}

