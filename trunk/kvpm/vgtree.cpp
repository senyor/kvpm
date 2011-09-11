/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 * * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include "vgtree.h"

#include <KConfigSkeleton>
#include <KLocale>
#include <QtGui>

#include "lvactionsmenu.h"
#include "logvol.h"
#include "masterlist.h"
#include "misc.h"
#include "topwindow.h"
#include "volgroup.h"

extern MasterList *master_list;


VGTree::VGTree(VolGroup *VolumeGroup) : QTreeWidget(), m_vg(VolumeGroup)
{
    QStringList header_labels;
    m_init = true;

    header_labels << "Volume" << "Size" << "Remaining" << "Filesystem" << "type" << "Stripes" << "Stripe size" 
		  << "Snap/Move" << "State" << "Access" << "Tags" << "Mount points";

    QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidgetItem *)0, header_labels);

    for(int column = 0; column < item->columnCount() ; column++)
        item->setTextAlignment(column, Qt::AlignCenter);

    item->setToolTip(0, i18n("Logical volume name"));
    item->setToolTip(1, i18n("Total size of the logical volume"));
    item->setToolTip(2, i18n("Free space on logical volume"));
    item->setToolTip(3, i18n("Filesystem type on logical volume, if any"));
    item->setToolTip(4, i18n("Type of logical volume"));
    item->setToolTip(5, i18n("Number of stripes if the volume is striped"));
    item->setToolTip(6, i18n("Size of stripes if the volume is striped"));
    item->setToolTip(7, i18n("Percentage of pvmove completed or percentage of snapshot used up"));
    item->setToolTip(8, i18n("Logical volume state"));
    item->setToolTip(9, i18n("Read and write or Read Only"));
    item->setToolTip(10, i18n("Optional tags for physical volume"));
    item->setToolTip(11, i18n("Filesystem mount points, if mounted"));

    setHeaderItem(item);
    sortByColumn(0, Qt::AscendingOrder);
}

void VGTree::loadData()
{
    QList<LogVol *> logical_volumes = m_vg->getLogicalVolumes();
    LogVol *lv = NULL;   
    QTreeWidgetItem *new_item;

    disconnect(this, SIGNAL(itemExpanded(QTreeWidgetItem *)), 
	    this, SLOT(adjustColumnWidth(QTreeWidgetItem *)));

    disconnect(this, SIGNAL(itemCollapsed(QTreeWidgetItem *)), 
	    this, SLOT(adjustColumnWidth(QTreeWidgetItem *)));

    setSortingEnabled(false);

    for(int x = 0; x < m_vg->getLogVolCount(); x++){
	
 	lv = logical_volumes[x];
        new_item = NULL;

        for(int y = topLevelItemCount() - 1; y >= 0; y--){
            if(topLevelItem(y)->data(0, Qt::DisplayRole).toString() == lv->getName())
                new_item = loadItem(lv, topLevelItem(y));
        }  

        if(new_item == NULL){
            new_item = new QTreeWidgetItem((QTreeWidgetItem *)0);
            addTopLevelItem(new_item);
            loadItem(lv, new_item);
        }

    }

    bool match;
    for(int y = topLevelItemCount() - 1; y >= 0; y--){ // remove top level lv items of deleted lvs
        match = false;
        for(int x = 0; x < logical_volumes.size(); x++){

            if(topLevelItem(y)->data(0, Qt::DisplayRole).toString() == logical_volumes[x]->getName() )
                match = true;
        }
        if( !match ){
            delete takeTopLevelItem(y);
        }
    }

    connect(this, SIGNAL(itemExpanded(QTreeWidgetItem *)), 
	    this, SLOT(adjustColumnWidth(QTreeWidgetItem *)));

    connect(this, SIGNAL(itemCollapsed(QTreeWidgetItem *)), 
	    this, SLOT(adjustColumnWidth(QTreeWidgetItem *)));

    if( currentItem() == NULL && topLevelItemCount() > 0 )
        setCurrentItem( topLevelItem(0) );

    if( currentItem() != NULL )
        setCurrentItem( currentItem() );

    setHiddenColumns();
    resizeColumnToContents(0);
    setSortingEnabled(true);
    setupContextMenu();
    m_init = false;

    return;
}

QTreeWidgetItem *VGTree::loadItem(LogVol *lv, QTreeWidgetItem *item)
{
    const QString old_type = item->data(4, Qt::DisplayRole).toString();  // lv type before reload or "" if new item
    const QString lv_name = lv->getName();
    const bool was_sc = old_type.contains("origin", Qt::CaseInsensitive);
    const bool is_sc  = lv->isSnapContainer();
    const int old_child_count = item->childCount();
    bool was_expanded = false;

    int new_child_count;
    QList<LogVol *> temp_kids;
    long long fs_remaining;       // remaining space on fs -- if known
    int fs_percent;               // percentage of space remaining

    if(is_sc && !was_sc)
        was_expanded = item->isExpanded();

    if(!is_sc && was_sc){
        for(int x = 0; x < old_child_count; x++){
            if( lv_name == item->child(x)->data(0, Qt::DisplayRole).toString() )
                was_expanded = item->child(x)->isExpanded();
        }
    }

    item->setData(0, Qt::DisplayRole, lv_name);
    item->setData(1, Qt::DisplayRole, sizeToString(lv->getSize()));           
      
    if( lv->getFilesystemSize() > -1 &&  lv->getFilesystemUsed() > -1 ){
        fs_remaining = lv->getFilesystemSize() - lv->getFilesystemUsed();
        fs_percent = qRound( ((double)fs_remaining / (double)lv->getFilesystemSize()) * 100 );
        item->setData(2, Qt::DisplayRole, QString(sizeToString(fs_remaining) + " (%%1)").arg(fs_percent) );
    }
    else
        item->setData(2, Qt::DisplayRole, QString(""));
    
    item->setData(3, Qt::DisplayRole, lv->getFilesystem() );
    item->setData(4, Qt::DisplayRole, lv->getType());

    if( lv->isPvmove() )
        item->setData(7, Qt::DisplayRole, QString("%%1").arg(lv->getCopyPercent(), 1, 'f', 2));
    else if( lv->isSnap() || lv->isMerging() )
        item->setData(7, Qt::DisplayRole, QString("%%1").arg(lv->getSnapPercent(), 1, 'f', 2));
    else
        item->setData(7, Qt::DisplayRole, QString(""));

    item->setData(8, Qt::DisplayRole, lv->getState());

    if(lv->isWritable())
        item->setData(9, Qt::DisplayRole, QString("r/w"));
    else
        item->setData(9, Qt::DisplayRole, QString("r/o"));

    item->setData(10, Qt::DisplayRole, lv->getTags().join(",")); 
    item->setData(11, Qt::DisplayRole, lv->getMountPoints().join(","));
    item->setData(0, Qt::UserRole, lv_name);

    if( lv->getSegmentCount() == 1 ) {
        item->setData(1, Qt::UserRole, 0);            // 0 means segment 0 data present

        if( lv->isMirror() ){
            item->setData(5, Qt::DisplayRole, QString(""));
            item->setData(6, Qt::DisplayRole, QString(""));
        }
        else{
            item->setData(5, Qt::DisplayRole, QString("%1").arg(lv->getSegmentStripes(0)) );
            item->setData(6, Qt::DisplayRole, sizeToString(lv->getSegmentStripeSize(0)) );
        }

        if( !is_sc && old_type.contains("origin", Qt::CaseInsensitive) ){
            for(int x = 0; x < old_child_count; x++){
                if( item->child(x)->data(0, Qt::DisplayRole) == lv->getName() )
                    item->setExpanded( item->child(x)->isExpanded() );
            }
        }

        insertChildItems(lv, item);
    }
    else {
        item->setData(1, Qt::UserRole, -1);            // -1 means not segment data
        item->setData(5, Qt::DisplayRole, QString(""));
        item->setData(6, Qt::DisplayRole, QString(""));
       
        insertSegmentItems(lv, item);
    }
    
    new_child_count = item->childCount();

    if( is_sc ){   // expand the item if it is a new snap container or snap count is different
        if(m_init){
            item->setExpanded(true);
        }
        else{
            if( !was_sc || old_child_count != new_child_count){
                item->setExpanded(true);
                if( !was_sc ){
                    for(int x = 0; x < new_child_count; x++){
                        if( item->child(x)->data(0, Qt::DisplayRole) == lv_name )
                            item->child(x)->setExpanded(was_expanded);
                    }
                }
            }
        }
    }
    else if( was_sc && !is_sc ){ // if it was formerly a snap container, set expanded to what the "real" lv was
        if( indexOfTopLevelItem(item) >= 0 )
            addTopLevelItem( takeTopLevelItem( indexOfTopLevelItem(item) ) ); // next line doesn't work without this!
        item->setExpanded(was_expanded);
    }

    for(int column = 1; column < item->columnCount() ; column++)
        item->setTextAlignment(column, Qt::AlignRight);

    return item;
}


void VGTree::setupContextMenu()
{
    setContextMenuPolicy(Qt::CustomContextMenu);   

    // disconnect the last connect, otherwise the following connect get repeated
    // and piles up.

    disconnect(this, SIGNAL(customContextMenuRequested(QPoint)), 
               this, SLOT(popupContextMenu(QPoint)) );

    if( !m_vg->isExported() ){

	connect(this, SIGNAL(customContextMenuRequested(QPoint)), 
		this, SLOT(popupContextMenu(QPoint)) );

    }
    return;
}

void VGTree::insertSegmentItems(LogVol *lv, QTreeWidgetItem *item)
{
    const int segment_count = lv->getSegmentCount();
    const int child_count = item->childCount();

    QStringList segment_data;
    QTreeWidgetItem *child_item;
    QList<QTreeWidgetItem *> segment_children;

    for(int x = 0; x < child_count ; x++)  // segments can never have children
        segment_children.append( item->child(x)->takeChildren() );

    for(int x = segment_children.size() - 1; x >= 0 ;x--)
        delete segment_children[x]; // so delete them

    if( segment_count > child_count ){
        for(int x = 0; x < segment_count - child_count; x++)
            new QTreeWidgetItem(item);
    }
    else if( segment_count < child_count ){
        for(int x = child_count - 1; x >= segment_count ; x--)
            delete (item->takeChild(x));
    }

    for(int x = 0; x < segment_count; x++){
            
        child_item = item->child(x);

        child_item->setData(0, Qt::DisplayRole, QString("Seg# %1").arg(x));
        child_item->setData(1, Qt::DisplayRole, sizeToString(lv->getSegmentSize(x)));           
        child_item->setData(2, Qt::DisplayRole, QString(""));
        child_item->setData(3, Qt::DisplayRole, QString(""));
        child_item->setData(4, Qt::DisplayRole, QString(""));
        child_item->setData(5, Qt::DisplayRole, QString("%1").arg(lv->getSegmentStripes(x)));
        child_item->setData(6, Qt::DisplayRole, sizeToString(lv->getSegmentStripeSize(x))); 
 	child_item->setData(0, Qt::UserRole, lv->getName());
	child_item->setData(1, Qt::UserRole, x);

        for(int x = 7; x < 12; x++)
            child_item->setData(x, Qt::DisplayRole, QString(""));
    
        for(int column = 1; column < child_item->columnCount() ; column++)
            child_item->setTextAlignment(column, Qt::AlignRight);
    }
}

//
// Change to parentVolume ????
// parentItem ????
void VGTree::insertChildItems(LogVol *originVolume, QTreeWidgetItem *item)
{
    QTreeWidgetItem *child_item;
    QStringList child_data;
    LogVol *child_volume;

    QList<LogVol *>  immediate_children = originVolume->getChildren();
    long lv_child_count = immediate_children.size();

    for(int x = 0; x < lv_child_count; x++){
        child_item = NULL;
	child_volume = immediate_children[x];

        for(int y = item->childCount() - 1; y >= 0; y--){
            if(item->child(y)->data(0, Qt::DisplayRole).toString() == child_volume->getName() )
                child_item = loadItem(child_volume, item->child(y));
        }

        if(child_item == NULL)
            child_item = loadItem(child_volume, new QTreeWidgetItem(item));
            
        for(int column = 1; column < child_item->columnCount() ; column++)
            child_item->setTextAlignment(column, Qt::AlignRight);
        
    }

    bool match;     // Remove child items for logical volumes that no longer exist
    for(int y = item->childCount() - 1; y >= 0; y--){
        match = false;
        for(int x = 0; x < immediate_children.size(); x++){
            child_volume = immediate_children[x];

            if(item->child(y)->data(0, Qt::DisplayRole).toString() == child_volume->getName() )
                match = true;
        }

        if( !match ){
            delete item->takeChild(y);
        }
    }

    return;
}

void VGTree::popupContextMenu(QPoint point)
{
    QTreeWidgetItem *item;
    KMenu *context_menu;
    LogVol *lv;
    QString lv_name;

    item = itemAt(point);
    if(item){                                 //item = 0 if there is no item a that point
	lv_name = QVariant(item->data(0, Qt::UserRole)).toString();
	lv = m_vg->getLogVolByName(lv_name);
	context_menu = new LVActionsMenu(lv, m_vg, this);
	context_menu->exec(QCursor::pos());
    }
    else{
	context_menu = new LVActionsMenu(NULL, m_vg, this);
	context_menu->exec(QCursor::pos());
    }
}

void VGTree::setHiddenColumns()
{
    KConfigSkeleton skeleton;

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
}

void VGTree::adjustColumnWidth(QTreeWidgetItem *)
{
    resizeColumnToContents(0);
}

