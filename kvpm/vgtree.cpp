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
#include <KIcon>
#include <KLocale>

#include <QtGui>

#include "lvactionsmenu.h"
#include "logvol.h"
#include "masterlist.h"
#include "misc.h"
#include "topwindow.h"
#include "volgroup.h"


VGTree::VGTree(VolGroup *VolumeGroup) : QTreeWidget(), m_vg(VolumeGroup)
{
    QStringList header_labels;
    m_init = true;

    header_labels << i18n("Volume")      << i18n("type")       << i18n("Size") 
                  << i18n("Remaining")   << i18n("Filesystem") << i18n("Stripes") 
                  << i18n("Stripe size") << i18n("Snap/Copy")  << i18n("State") 
                  << i18n("Access")      << i18n("Tags")       << i18n("Mount points");

    QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidgetItem *)0, header_labels);

    for(int column = 0; column < item->columnCount() ; column++)
        item->setTextAlignment(column, Qt::AlignCenter);

    item->setToolTip(0, i18n("Logical volume name"));
    item->setToolTip(1, i18n("Type of logical volume"));
    item->setToolTip(2, i18n("Total size of the logical volume"));
    item->setToolTip(3, i18n("Free space on logical volume"));
    item->setToolTip(4, i18n("Filesystem type on logical volume, if any"));
    item->setToolTip(5, i18n("Number of stripes if the volume is striped"));
    item->setToolTip(6, i18n("Size of stripes if the volume is striped"));
    item->setToolTip(7, i18n("Percentage of pvmove completed, of mirror synced or of snapshot used up"));
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
    setViewConfig();

    for(int x = 0; x < m_vg->getLVCount(); x++){
	
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

    for(int x = 0; x < 10; x++)
        resizeColumnToContents(x);

    setSortingEnabled(true);
    setupContextMenu();
    m_init = false;
    emit currentItemChanged(currentItem(), currentItem());

    return;
}

QTreeWidgetItem *VGTree::loadItem(LogVol *lv, QTreeWidgetItem *item)
{
    const QString old_type = item->data(1, Qt::DisplayRole).toString();  // lv type before reload or "" if new item
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

    if( lv->hasMissingVolume() ){
        item->setIcon(0, KIcon("exclamation"));
        item->setToolTip(0, i18n("one or more physical volumes are missing"));
    }
    else if( !lv->isSnapContainer() && lv->isOrigin() ){
        item->setIcon(0, KIcon("bullet_star"));
        item->setToolTip(0, i18n("origin"));
    }
    else{
        item->setIcon(0, KIcon());
        item->setToolTip(0, QString());
    }

    item->setData(1, Qt::DisplayRole, lv->getType());

    if(lv->isSnapContainer())
        item->setData(2, Qt::DisplayRole, sizeToString(lv->getTotalSize()));           
    else{
        item->setData(2, Qt::DisplayRole, sizeToString(lv->getSize()));           
      
        if( lv->getFilesystemSize() > -1 &&  lv->getFilesystemUsed() > -1 ){

            fs_remaining = lv->getFilesystemSize() - lv->getFilesystemUsed();
            fs_percent = qRound( ((double)fs_remaining / (double)lv->getFilesystemSize()) * 100 );
            
            if(m_show_total && !m_show_percent)
                item->setData(3, Qt::DisplayRole, sizeToString(fs_remaining));
            else if(!m_show_total && m_show_percent)
                item->setData(3, Qt::DisplayRole, QString("%%1").arg(fs_percent) );
            else
                item->setData(3, Qt::DisplayRole, QString(sizeToString(fs_remaining) + " (%%1)").arg(fs_percent) );
            
            if( fs_percent <= m_fs_warn_percent ){
                item->setIcon(3, KIcon("exclamation"));
                item->setToolTip(3, i18n("This filesystem is running out of space"));
            }
            else{
                item->setIcon(3, KIcon());
                item->setToolTip(3, QString());
            }
        }
        else{
            item->setData(3, Qt::DisplayRole, QString(""));
            item->setIcon(3, KIcon());
            item->setToolTip(3, QString());
        }
        
        item->setData(4, Qt::DisplayRole, lv->getFilesystem() );
        
        if( (lv->isPvmove() || lv->isMirror()) && !lv->isSnapContainer() )
            item->setData(7, Qt::DisplayRole, QString("%%1").arg(lv->getCopyPercent(), 1, 'f', 2));
        else if( lv->isSnap() || lv->isMerging() )
            item->setData(7, Qt::DisplayRole, QString("%%1").arg(lv->getSnapPercent(), 1, 'f', 2));
        else
            item->setData(7, Qt::DisplayRole, QString(""));
        
        item->setData(8, Qt::DisplayRole, lv->getState());
        
        if( !lv->isSnapContainer() && !lv->isMirrorLog() && !lv->isMirrorLeg() && !lv->isVirtual() ){
            if( lv->isMounted() ){
                item->setIcon( 8, KIcon("emblem-mounted") );
                item->setToolTip( 8, i18n("mounted filesystem") );
            }
            else{
                item->setIcon( 8, KIcon("emblem-unmounted") );
                item->setToolTip( 8, i18n("unmounted filesystem") );
            }
        }
        
        if(lv->isWritable())
            item->setData(9, Qt::DisplayRole, QString("r/w"));
        else
            item->setData(9, Qt::DisplayRole, QString("r/o"));
        
        item->setData(10, Qt::DisplayRole, lv->getTags().join(",")); 
        item->setData(11, Qt::DisplayRole, lv->getMountPoints().join(","));
    }

    item->setData(0, Qt::UserRole, lv_name);
    item->setData(2, Qt::UserRole, lv->getUuid());
    
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

    if(is_sc){   // expand the item if it is a new snap container or snap count is different

        if(m_init){
            item->setExpanded(true);
        }
        else{
            if( !was_sc || old_child_count != new_child_count ){
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

        if( lv->getPVNames(x).contains("unknown device") ){
            child_item->setIcon(0, KIcon("exclamation") );
            child_item->setToolTip( 0, i18n("one or more physical volumes are missing") );
        }
        else{
            child_item->setIcon(0, KIcon() );
            child_item->setToolTip( 0, QString() );
        }

        child_item->setData(0, Qt::DisplayRole, QString("Seg# %1").arg(x));
        child_item->setData(1, Qt::DisplayRole, QString(""));
        child_item->setData(2, Qt::DisplayRole, sizeToString(lv->getSegmentSize(x)));           
        child_item->setData(3, Qt::DisplayRole, QString(""));
        child_item->setData(4, Qt::DisplayRole, QString(""));
        child_item->setData(5, Qt::DisplayRole, QString("%1").arg(lv->getSegmentStripes(x)));
        child_item->setData(6, Qt::DisplayRole, sizeToString(lv->getSegmentStripeSize(x))); 
 	child_item->setData(0, Qt::UserRole, lv->getName());
	child_item->setData(1, Qt::UserRole, x);
 	child_item->setData(2, Qt::UserRole, lv->getUuid());
 	child_item->setData(3, Qt::UserRole, QString("segment"));

        for(int x = 7; x < 12; x++)
            child_item->setData(x, Qt::DisplayRole, QString(""));
    
        for(int column = 1; column < child_item->columnCount() ; column++)
            child_item->setTextAlignment(column, Qt::AlignRight);
    }
}

void VGTree::insertChildItems(LogVol *parentVolume, QTreeWidgetItem *parentItem)
{
    QTreeWidgetItem *child_item;
    QStringList child_data;
    LogVol *child_volume;

    QList<LogVol *>  immediate_children = parentVolume->getChildren();
    long lv_child_count = immediate_children.size();

    for(int x = 0; x < lv_child_count; x++){
        child_item = NULL;
	child_volume = immediate_children[x];

        for(int y = parentItem->childCount() - 1; y >= 0; y--){
            if(parentItem->child(y)->data(0, Qt::DisplayRole).toString() == child_volume->getName() )
                child_item = loadItem(child_volume, parentItem->child(y));
        }

        if(child_item == NULL){
            child_item = loadItem(child_volume, new QTreeWidgetItem(parentItem));
        }

        for(int column = 1; column < child_item->columnCount() ; column++)
            child_item->setTextAlignment(column, Qt::AlignRight);
    }

    bool match;     // Remove child items for logical volumes that no longer exist
    for(int y = parentItem->childCount() - 1; y >= 0; y--){

        match = false;

        for(int x = 0; x < immediate_children.size(); x++){
            child_volume = immediate_children[x];

            if(parentItem->child(y)->data(0, Qt::DisplayRole).toString() == child_volume->getName() ){
                match = true;
            }
        }

        if( !match ){
            delete parentItem->takeChild(y);
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
    int segment;    // segment = -1 means whole lv

    item = itemAt(point);
    if(item){                                 //item = 0 if there is no item a that point
	lv_name = QVariant(item->data(0, Qt::UserRole)).toString();
	lv = m_vg->getLVByName(lv_name);

        if( QVariant(item->data(3, Qt::UserRole) ).toString() == "segment" )
            segment = QVariant(item->data(1, Qt::UserRole)).toInt();
        else
            segment = -1;

	context_menu = new LVActionsMenu(lv, segment, m_vg, this);
	context_menu->exec(QCursor::pos());
    }
    else{
	context_menu = new LVActionsMenu(NULL, 0, m_vg, this);
	context_menu->exec(QCursor::pos());
    }
}

void VGTree::setViewConfig()
{
    KConfigSkeleton skeleton;

    bool changed = false;

    bool volume,      size,      remaining,
         filesystem,  type,
         stripes,     stripesize,
         snapmove,    state,
         access,      tags,
         mountpoints;

    skeleton.setCurrentGroup("AllTreeColumns");
    skeleton.addItemBool( "percent", m_show_percent );
    skeleton.addItemBool( "total",   m_show_total );
    skeleton.addItemInt(  "fs_warn", m_fs_warn_percent );

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
        setColumnHidden( 1, !type );
        setColumnHidden( 2, !size );
        setColumnHidden( 3, !remaining );
        setColumnHidden( 4, !filesystem );
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

void VGTree::adjustColumnWidth(QTreeWidgetItem *)
{
    resizeColumnToContents(0);
    resizeColumnToContents(1);
    resizeColumnToContents(6);
}

