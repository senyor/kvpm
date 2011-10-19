/*
 *
 * 
 * Copyright (C) 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 * * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include "devicetree.h"

#include <KConfigSkeleton>
#include <KIcon>
#include <KLocale>

#include <QtGui>

#include "deviceactionsmenu.h"
#include "misc.h"
#include "physvol.h"
#include "storagedevice.h"
#include "storagepartition.h"
#include "volgroup.h"


DeviceTree::DeviceTree(QList<StorageDevice *> devices, QWidget *parent) : QTreeWidget(parent)
{
    QStringList header_labels;

    header_labels << "Device"    << "Type"  << "Capacity" 
                  << "Remaining" << "Usage" << "Group"
                  << "Flags"     << "Mount point" ;
    
    QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidgetItem *)0, header_labels);

    for(int column = 0; column < item->columnCount() ; column++)
        item->setTextAlignment(column, Qt::AlignCenter);

    item->setToolTip(0, i18n("Device name"));
    item->setToolTip(1, i18n("Total size of the logical volume"));
    item->setToolTip(2, i18n("Size of storage space"));
    item->setToolTip(3, i18n("Remaining storage space"));
    item->setToolTip(4, i18n("How the device is being used"));
    item->setToolTip(5, i18n("Name of volume group if device is a physical volume"));
    item->setToolTip(6, i18n("Flags associated with device"));
    item->setToolTip(7, i18n("Mount points of filesystem if mounted"));

    setHeaderItem(item);
    loadData(devices);

    expandAll();
    setAlternatingRowColors(true); 
    resizeColumnToContents(0);
    resizeColumnToContents(3);
    resizeColumnToContents(5);
    setAllColumnsShowFocus(true);
    setExpandsOnDoubleClick(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
}

void DeviceTree::loadData(QList<StorageDevice *> devices)
{

    QTreeWidgetItem *parent, *child, *extended = 0;
    StorageDevice *dev;
    QStringList data;
    StoragePartition *part;
    QString type;
    QVariant part_variant;
    QVariant dev_variant;
    PhysVol *pv;
    
    /* 
       item->data(x, Qt::UserRole)
       x = 0:  pointer to storagepartition if partition, else "" 
       x = 1:  pointer to storagedevice
    */
    
    clear();

    for(int x = 0; x < devices.size(); x++){
        data.clear();
        dev = devices[x];
        dev_variant.setValue( (void *) dev);
        if(dev->isPhysicalVolume()){
            pv = dev->getPhysicalVolume();
            data << dev->getName() << "" << sizeToString(dev->getSize());
            data << QString("%1 (%%2) ").arg( sizeToString( (pv->getSize()) - (pv->getUnused()) )).arg(pv->getPercentUsed() );
            data << "PV" << pv->getVG()->getName();
        }
        else{
            data << dev->getName() << "" << sizeToString(dev->getSize());
        }
        
        parent = new QTreeWidgetItem(data);
        parent->setData(1, Qt::UserRole, dev_variant);
        parent->setTextAlignment(2, Qt::AlignRight);
        parent->setTextAlignment(3, Qt::AlignRight);

        if( dev->isPhysicalVolume() ){
            if( dev->getPhysicalVolume()->isActive() ){
                parent->setIcon(4, KIcon("lightbulb"));
                parent->setToolTip(4, i18n("Physical volume with active logical volumes"));
            }
            else{
                parent->setIcon(4, KIcon("lightbulb_off"));
                parent->setToolTip(4, i18n("Physical volume without active logical volumes"));
            }
        }

        addTopLevelItem(parent);

        for(int y = 0; y < dev->getPartitionCount(); y++){
            data.clear();

            part = dev->getStoragePartitions()[y];
            part_variant.setValue( (void *) part);
            type = part->getType();
            
            data << part->getName() << type << sizeToString(part->getSize());
            
            if(part->isPhysicalVolume()){
                pv = part->getPhysicalVolume();
                data << QString("%1 (%%2) ").arg( sizeToString( (pv->getUnused()))).arg(100 - pv->getPercentUsed() )
                     << "PV"
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
                extended = new QTreeWidgetItem(data);
                extended->setData(0, Qt::UserRole, part_variant);
                extended->setData(1, Qt::UserRole, dev_variant);
                extended->setTextAlignment(2, Qt::AlignRight);
                extended->setTextAlignment(3, Qt::AlignRight);
                parent->addChild(extended);
            }
            else if( !( (type == "logical") || (type == "freespace (logical)") ) ){
                child = new QTreeWidgetItem(data);
                child->setData(0, Qt::UserRole, part_variant);
                child->setData(1, Qt::UserRole, dev_variant);
                child->setTextAlignment(2, Qt::AlignRight);
                child->setTextAlignment(3, Qt::AlignRight);

                if( part->isPhysicalVolume() ){
                    if( part->getPhysicalVolume()->isActive() ){
                        child->setIcon(4, KIcon("lightbulb"));
                        child->setToolTip(4, i18n("Physical volume with active logical volumes"));
                    }
                    else{
                        child->setIcon(4, KIcon("lightbulb_off"));
                        child->setToolTip(4, i18n("Physical volume without active logical volumes"));
                    }
                }
                else if( part->isMountable() ){
                    if( part->isMounted() ){
                        child->setIcon(4, KIcon("emblem-mounted"));
                        child->setToolTip(4, i18n("mounted filesystem") );
                    }
                    else{
                        child->setIcon(4, KIcon("emblem-unmounted"));
                        child->setToolTip(4, i18n("unmounted filesystem"));
                    }
                }

                parent->addChild(child);
            }
            else if(extended){
                child = new QTreeWidgetItem(data);
                child->setData(0, Qt::UserRole, part_variant);
                child->setData(1, Qt::UserRole, dev_variant);
                child->setTextAlignment(2, Qt::AlignRight);
                child->setTextAlignment(3, Qt::AlignRight);

                if( part->isPhysicalVolume() ){
                    if( part->getPhysicalVolume()->isActive() ){
                        child->setIcon(4, KIcon("lightbulb"));
                        child->setToolTip(4, i18n("Physical volume with active logical volumes"));
                    }
                    else{
                        child->setIcon(4, KIcon("lightbulb_off"));
                        child->setToolTip(4, i18n("Physical volume without active logical volumes"));
                    }
                }
                else if( part->isMountable() ){
                    if( part->isMounted() ){
                        child->setIcon(4, KIcon("emblem-mounted"));
                        child->setToolTip(4, i18n("mounted filesystem") );
                    }
                    else{
                        child->setIcon(4, KIcon("emblem-unmounted"));
                        child->setToolTip(4, i18n("unmounted filesystem"));
                    }
                }

                extended->addChild(child);
            }
        }
    }

    setupContextMenu();

    if( topLevelItemCount() && ( currentItem() == NULL ) )
        setCurrentItem( topLevelItem(0) );

    return;
}

void DeviceTree::setupContextMenu()
{
    setContextMenuPolicy(Qt::CustomContextMenu);   

    // disconnect the last connect, otherwise the following connect get repeated
    // and piles up.

    disconnect(this, SIGNAL(customContextMenuRequested(QPoint)), 
               this, SLOT(popupContextMenu(QPoint)) );

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), 
            this, SLOT(popupContextMenu(QPoint)) );

    return;
}

void DeviceTree::popupContextMenu(QPoint point)
{
    QTreeWidgetItem *const item = itemAt(point); 
    KMenu *context_menu;

    if(item){                          
        context_menu = new DeviceActionsMenu(item, this);
        context_menu->exec(QCursor::pos());
    }
}

void DeviceTree::setHiddenColumns()
{
    KConfigSkeleton skeleton;

    bool changed = false;

    bool volume,      size,      remaining,
         filesystem,  type,
         stripes,     stripesize,
         snapmove,    state,
         access,      tags,
         mountpoints;

    skeleton.setCurrentGroup("VolumeTreeColumns");
    skeleton.addItemBool( "volume",      volume );
    skeleton.addItemBool( "size",        size );
    skeleton.addItemBool( "remaining",   remaining );
    skeleton.addItemBool( "filesystem",  filesystem );
    skeleton.addItemBool( "type",        type );
    skeleton.addItemBool( "stripes",     stripes );
    skeleton.addItemBool( "stripesize",  stripesize );
    skeleton.addItemBool( "snapmove",    snapmove );
    skeleton.addItemBool( "state",       state );
    skeleton.addItemBool( "access",      access );
    skeleton.addItemBool( "tags",        tags );
    skeleton.addItemBool( "mountpoints", mountpoints );

    if( !( !volume == isColumnHidden(0)     && !size == isColumnHidden(1) &&
           !remaining == isColumnHidden(2)  && !filesystem == isColumnHidden(3) &&
           !type == isColumnHidden(4)       && !stripes == isColumnHidden(5) &&
           !stripesize == isColumnHidden(6) && !snapmove == isColumnHidden(7) &&
           !state == isColumnHidden(8)      && !access == isColumnHidden(9) &&
           !tags == isColumnHidden(10)      && !mountpoints == isColumnHidden(11) ) )
        changed = true;

    if(changed){

        setColumnHidden( 0, !volume );
        setColumnHidden( 1, !size );
        setColumnHidden( 2, !remaining );
        setColumnHidden( 3, !filesystem );
        setColumnHidden( 4, !type );
        setColumnHidden( 5, !stripes );
        setColumnHidden( 6, !stripesize );
        setColumnHidden( 7, !snapmove );
        setColumnHidden( 8, !state );
        setColumnHidden( 9, !access );
        setColumnHidden( 10, !tags );
        setColumnHidden( 11, !mountpoints );

        for(int column = 0; column < 11; column++){
            if( !isColumnHidden(column) )
                resizeColumnToContents(column);
        }
    }
}

