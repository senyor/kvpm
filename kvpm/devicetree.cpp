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
#include "devicepropertiesstack.h"
#include "devicesizechart.h"
#include "misc.h"
#include "physvol.h"
#include "storagedevice.h"
#include "storagepartition.h"
#include "volgroup.h"


DeviceTree::DeviceTree(DeviceSizeChart *const chart, DevicePropertiesStack *const stack, QWidget *parent)
 : QTreeWidget(parent), 
   m_chart(chart),
   m_stack(stack)
{
    QStringList header_labels;

    header_labels << "Device"    << "Type"  << "Capacity" 
                  << "Remaining" << "Usage" << "Group"
                  << "Flags"     << "Mount point" ;
    
    QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidgetItem *)0, header_labels);

    for(int column = 0; column < item->columnCount() ; column++)
        item->setTextAlignment(column, Qt::AlignCenter);

    item->setToolTip(0, i18n("The device name"));
    item->setToolTip(1, i18n("The type of partition"));
    item->setToolTip(2, i18n("The amount of storage space"));
    item->setToolTip(3, i18n("The remaining storage space"));
    item->setToolTip(4, i18n("How the device is being used"));
    item->setToolTip(5, i18n("The Name of the volume group if the device is a physical volume"));
    item->setToolTip(6, i18n("Any flags associated with device"));
    item->setToolTip(7, i18n("Mount points of the filesystem if it is mounted"));

    m_initial_run = true;
    setHeaderItem(item);
    setAlternatingRowColors(true); 
    setAllColumnsShowFocus(true);
    setExpandsOnDoubleClick(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setupContextMenu();
}

void DeviceTree::loadData(QList<StorageDevice *> devices)
{
    QTreeWidgetItem  *parent, *child, *extended = 0;
    StoragePartition *part;
    StorageDevice *dev;
    PhysVol *pv;
    QStringList data, expanded_items;
    QString dev_name, part_name, type, current_device;
    QVariant part_variant, dev_variant;

    /* 
       item->data(x, Qt::UserRole)
       x = 0:  pointer to storagepartition if partition, else "" 
       x = 1:  pointer to storagedevice
    */

    disconnect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem *)), 
               m_chart, SLOT(setNewDevice(QTreeWidgetItem*)));

    disconnect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), 
               m_stack, SLOT(changeDeviceStackIndex(QTreeWidgetItem*)));

    setViewConfig();

    if( currentItem() )
        current_device = currentItem()->data(0, Qt::DisplayRole).toString();

    for(int x = topLevelItemCount() - 1; x >= 0; x--){
        parent = topLevelItem(x);

        if( parent->isExpanded() )
            expanded_items << parent->data(0, Qt::DisplayRole).toString();

        for(int y = parent->childCount() - 1; y >= 0; y--){ 
            if( parent->child(y)->isExpanded() )
                expanded_items << parent->child(y)->data(0, Qt::DisplayRole).toString();
        }

        delete takeTopLevelItem(x);
    }

    for(int x = 0; x < devices.size(); x++){
        data.clear();
        dev = devices[x];
        dev_variant.setValue( (void *) dev);
        dev_name = dev->getName();

        if(dev->isPhysicalVolume()){
            pv = dev->getPhysicalVolume();
            data << dev_name << "" << sizeToString(dev->getSize());

            if(m_show_total && !m_show_percent)
                data << sizeToString( pv->getRemaining() );
            else if(!m_show_total && m_show_percent)
                data << QString("%%1").arg( 100 - pv->getPercentUsed() );
            else
                data << QString("%1 (%%2) ").arg( sizeToString( pv->getRemaining() )).arg( 100 - pv->getPercentUsed() );

            data << "PV" << pv->getVg()->getName();
        }
        else{
            data << dev_name << "" << sizeToString(dev->getSize());
        }
        
        parent = new QTreeWidgetItem(data);
        parent->setData(1, Qt::UserRole, dev_variant);
        parent->setTextAlignment(2, Qt::AlignRight);
        parent->setTextAlignment(3, Qt::AlignRight);

        if( dev->isPhysicalVolume() ){
            if( m_pv_warn_percent && ( m_pv_warn_percent >= (100 - dev->getPhysicalVolume()->getPercentUsed() ) ) ){
                parent->setIcon(3, KIcon("exclamation"));
                parent->setToolTip(3, i18n("Physical volume that is running out of space"));
            }

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

        for(int n = expanded_items.size() - 1; n >= 0; n--){ // if the old item for this dev was expanded, expand new one
            if(expanded_items[n] == dev_name){
                parent->setExpanded(true);
                expanded_items.removeAt(n);
                break;
            }
        }

        for(int y = 0; y < dev->getPartitionCount(); y++){
            data.clear();

            part = dev->getStoragePartitions()[y];
            part_variant.setValue( (void *) part);
            type = part->getType();
            
            data << part->getName() << type << sizeToString(part->getSize());
            
            if(part->isPhysicalVolume()){
                pv = part->getPhysicalVolume();

                if(m_show_total && !m_show_percent)
                    data << sizeToString( pv->getRemaining() );
                else if(!m_show_total && m_show_percent)
                    data << QString("%%1").arg( 100 - pv->getPercentUsed() );
                else
                    data << QString("%1 (%%2) ").arg( sizeToString( pv->getRemaining() )).arg( 100 - pv->getPercentUsed() );

                data << "PV"
                     << pv->getVg()->getName()
                     << (part->getFlags()).join(", ") 
                     << "";
            }
            else{
                if(part->getFilesystemSize() > -1 && part->getFilesystemUsed() > -1){

                if(m_show_total && !m_show_percent)
                    data << sizeToString( part->getFilesystemRemaining() );
                else if(!m_show_total && m_show_percent)
                    data << QString("%%1").arg( 100 - part->getFilesystemPercentUsed() );
                else
                    data << QString("%1 (%%2) ").arg( sizeToString( part->getFilesystemRemaining() )).arg( 100 - part->getFilesystemPercentUsed() );
                }
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
                part_name = part->getName();
    
                for(int n = expanded_items.size() - 1; n >= 0; n--){ // if the old item for this part was expanded, expand new one
                    if(expanded_items[n] == part_name){
                        extended->setExpanded(true);
                        expanded_items.removeAt(n);
                        break;
                    }
                }
            }
            else if( !( (type == "logical") || (type == "freespace (logical)") ) ){
                child = new QTreeWidgetItem(data);
                child->setData(0, Qt::UserRole, part_variant);
                child->setData(1, Qt::UserRole, dev_variant);
                child->setTextAlignment(2, Qt::AlignRight);
                child->setTextAlignment(3, Qt::AlignRight);

                if( part->isPhysicalVolume() ){
                    if( m_pv_warn_percent && ( m_pv_warn_percent >= (100 - part->getPhysicalVolume()->getPercentUsed() ) ) ){
                        child->setIcon(3, KIcon("exclamation"));
                        child->setToolTip(3, i18n("Physical volume that is running out of space"));
                    }

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
                        if( m_fs_warn_percent && ( m_fs_warn_percent >= (100 - part->getFilesystemPercentUsed() ) ) ){
                            child->setIcon(3, KIcon("exclamation"));
                            child->setToolTip(3, i18n("Filesystem that is running out of space"));
                        }

                        child->setIcon(4, KIcon("emblem-mounted"));
                        child->setToolTip(4, i18n("mounted filesystem") );
                    }
                    else{
                        child->setIcon(4, KIcon("emblem-unmounted"));
                        child->setToolTip(4, i18n("unmounted filesystem"));
                    }
                }
                else if( part->getFilesystem() == "swap" ){
                    if( part->isBusy() ){
                        child->setIcon(4, KIcon("task-recurring"));
                        child->setToolTip(4, i18n("Active swap area") );
                    }
                    else{
                        child->setIcon(4, KIcon("emblem-unmounted"));
                        child->setToolTip(4, i18n("Inactive swap area"));
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
                    if( m_pv_warn_percent && ( m_pv_warn_percent >= (100 - part->getPhysicalVolume()->getPercentUsed() ) ) ){
                        child->setIcon(3, KIcon("exclamation"));
                        child->setToolTip(3, i18n("Physical volume that is running out of space"));
                    }

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
                        if( m_fs_warn_percent && ( m_fs_warn_percent >= (100 - part->getFilesystemPercentUsed() ) ) ){
                            child->setIcon(3, KIcon("exclamation"));
                            child->setToolTip(3, i18n("Filesystem that is running out of space"));
                        }

                        child->setIcon(4, KIcon("emblem-mounted"));
                        child->setToolTip(4, i18n("mounted filesystem") );
                    }
                    else{
                        child->setIcon(4, KIcon("emblem-unmounted"));
                        child->setToolTip(4, i18n("unmounted filesystem"));
                    }
                }
                else if( part->getFilesystem() == "swap" ){
                    if( part->isBusy() ){
                        child->setIcon(4, KIcon("task-recurring"));
                        child->setToolTip(4, i18n("Active swap area") );
                    }
                    else{
                        child->setIcon(4, KIcon("emblem-unmounted"));
                        child->setToolTip(4, i18n("Inactive swap area"));
                    }
                }

                extended->addChild(child);
            }
        }
    }

    bool match = false;

    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem *)), 
            m_chart, SLOT(setNewDevice(QTreeWidgetItem*)));

    connect(this, SIGNAL( currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*) ), 
	    m_stack, SLOT( changeDeviceStackIndex(QTreeWidgetItem*) ));

    if(m_initial_run){
        m_initial_run = false;
        if(m_expand_parts)
            expandAll();
    }
    else{
        for(int x = topLevelItemCount() - 1; x >= 0; x--){
            parent = topLevelItem(x);
            if( parent->data(0, Qt::DisplayRole).toString() == current_device ){
                match = true;
                setCurrentItem(parent);
                break;
            }
            for(int y = parent->childCount() - 1; y >= 0; y--){
                child = parent->child(y);
                if( child->data(0, Qt::DisplayRole).toString() == current_device ){
                    match = true;
                    setCurrentItem(child);
                    break;
                }
                for(int z = child->childCount() - 1; z >= 0; z--){
                    if( child->child(z)->data(0, Qt::DisplayRole).toString() == current_device ){
                        match = true;
                        setCurrentItem(child->child(z));
                        break;
                    }
                }
                if(match)
                    break;
            }
            if(match)
                break;
        }
    }

    resizeColumnToContents(0);
    resizeColumnToContents(3);
    resizeColumnToContents(5);

    if( topLevelItemCount() && ( currentItem() == NULL ) )
        setCurrentItem( topLevelItem(0) );

    return;
}

void DeviceTree::setupContextMenu()
{
    setContextMenuPolicy(Qt::CustomContextMenu);   

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

void DeviceTree::setViewConfig()
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
    skeleton.addItemBool( "device",      device );
    skeleton.addItemBool( "partition",   partition );
    skeleton.addItemBool( "capacity",    capacity );
    skeleton.addItemBool( "remaining",   remaining );
    skeleton.addItemBool( "usage",       usage );
    skeleton.addItemBool( "group",       group );
    skeleton.addItemBool( "flags",       flags );
    skeleton.addItemBool( "mount",       mount );
    skeleton.addItemBool( "expandparts", m_expand_parts );

    skeleton.setCurrentGroup("AllTreeColumns");
    skeleton.addItemBool( "total",   m_show_total );
    skeleton.addItemBool( "percent", m_show_percent );
    skeleton.addItemInt(  "fs_warn", m_fs_warn_percent );
    skeleton.addItemInt(  "pv_warn", m_pv_warn_percent );

    setColumnHidden( 0, !device );
    setColumnHidden( 1, !partition );
    setColumnHidden( 2, !capacity );
    setColumnHidden( 3, !remaining );
    setColumnHidden( 4, !usage );
    setColumnHidden( 5, !group );
    setColumnHidden( 6, !flags );
    setColumnHidden( 7, !mount );
}
