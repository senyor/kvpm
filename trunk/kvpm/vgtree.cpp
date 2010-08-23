/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 * * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include <KConfigSkeleton>

#include <QtGui>

#include "lvactionsmenu.h"
#include "logvol.h"
#include "masterlist.h"
#include "misc.h"
#include "topwindow.h"
#include "vgtree.h"
#include "volgroup.h"

extern MasterList *master_list;


VGTree::VGTree(VolGroup *VolumeGroup) : QTreeWidget(), m_vg(VolumeGroup)
{
    QStringList header_labels;

    m_vg_name = m_vg->getName();
    setColumnCount(9);

    header_labels << "Volume" << "Size" << "Remaining" << "Filesystem" << "type" << "Stripes" << "Stripe size" 
		  << "Snap/Move" << "State" << "Access" << "Tags" << "Mount points";

    setHeaderLabels(header_labels);
}

void VGTree::loadData()
{
    QList<LogVol *> logical_volumes = m_vg->getLogicalVolumes();
    LogVol *lv = NULL;   
    QTreeWidgetItem *lv_item = NULL;
    QTreeWidgetItem *current_item = currentItem(); // The current item in the tree we are about to clear
    bool current_is_expanded = false;              // Is that current item expanded
    QStringList lv_data;
    QString current_lv_name;
    long long fs_remaining;       // remaining space on fs -- if known
    int fs_percent;               // percentage of space remaining

    disconnect(this, SIGNAL(itemExpanded(QTreeWidgetItem *)), 
	    this, SLOT(adjustColumnWidth(QTreeWidgetItem *)));

    disconnect(this, SIGNAL(itemCollapsed(QTreeWidgetItem *)), 
	    this, SLOT(adjustColumnWidth(QTreeWidgetItem *)));

    // Get the currently selected item so it can be reselected
    // after the tree is cleared and all the new data is loaded.

    if( m_lv_tree_items.size() && current_item ){
        current_is_expanded = current_item->isExpanded();
        current_lv_name   = ( current_item->data(0, Qt::UserRole) ).toString();
    }
    else{
        current_is_expanded = false;
        current_lv_name = "";
    }

    clear();
    m_lv_tree_items.clear();
    current_item = NULL;
    setupContextMenu();

    for(int x = 0; x < m_vg->getLogVolCount(); x++){
	
	lv = logical_volumes[x];
	lv_data.clear();

	if( ( !lv->isMirrorLeg() ) && ( !lv->isMirrorLog() ) &&
	    ( !(lv->isVirtual() && ( !lv->isOrphan() )) ) &&
	    ( !(lv->isMirror() && lv->getOrigin() != "" ) ) ){
	    
	    if( lv->getSegmentCount() == 1 ) {
		lv_data << lv->getName() << sizeToString(lv->getSize());

                if( lv->getFilesystemSize() > -1 &&  lv->getFilesystemUsed() > -1 ){
                    fs_remaining = lv->getFilesystemSize() - lv->getFilesystemUsed();
                    fs_percent = qRound( ((double)fs_remaining / (double)lv->getFilesystemSize()) * 100 );
                    lv_data << QString(sizeToString(fs_remaining) + " (%%1)").arg(fs_percent);
                }
                else
                    lv_data << "";

                lv_data << lv->getFilesystem() << lv->getType(); 

                if( lv->isMirror() )
                    lv_data << "" << "";
		else
                    lv_data << QString("%1").arg(lv->getSegmentStripes(0)) << sizeToString(lv->getSegmentStripeSize(0));

		if( lv->isSnap() )
		    lv_data    << QString("%%1").arg(lv->getSnapPercent());
		else if( lv->isPvmove() )
		    lv_data    << QString("%%1").arg(lv->getCopyPercent());
		else
		    lv_data << " ";
		
		lv_data << lv->getState();

		if(lv->isWritable())
		    lv_data << "r/w";
		else
		    lv_data << "r/o";

		lv_data << lv->getTags().join(",") << lv->getMountPoints().join(",");

		lv_item = new QTreeWidgetItem((QTreeWidgetItem *)0, lv_data);
		m_lv_tree_items.append(lv_item);
		lv_item->setData(0, Qt::UserRole, lv->getName());
		lv_item->setData(1, Qt::UserRole, 0);            // 0 means segment 0 data present

		if((lv->isMirror() && lv->getOrigin() == "" ) || 
		   (lv->isVirtual() && lv->getOrigin() == "" ) || 
		    lv->isUnderConversion() ){

		    insertMirrorLegItems(lv, lv_item);
		}
	    }
	    else {
		lv_data << lv->getName() << sizeToString(lv->getSize());

                if( lv->getFilesystemSize() > -1 &&  lv->getFilesystemUsed() > -1 ){
                    fs_remaining = lv->getFilesystemSize() - lv->getFilesystemUsed();
                    fs_percent = qRound( ((double)fs_remaining / (double)lv->getFilesystemSize()) * 100 );
                    lv_data << QString(sizeToString(fs_remaining) + " (%%1)").arg(fs_percent);
                }
                else
                    lv_data << "";

                lv_data << lv->getFilesystem() << lv->getType() << "" << "";
		
		if( lv->isSnap() )
		    lv_data    << QString("%%1").arg(lv->getSnapPercent());
		else if( lv->isPvmove() )
		    lv_data    << QString("%%1").arg(lv->getCopyPercent());
		else
		    lv_data << " ";
		
		lv_data << lv->getState();
		
		if(lv->isWritable())
		    lv_data << "r/w";
		else
		    lv_data << "r/o";

		lv_data << lv->getTags().join(",") << lv->getMountPoints().join(",");
	
		lv_item = new QTreeWidgetItem((QTreeWidgetItem *)0, lv_data);
		lv_item->setData(0, Qt::UserRole, lv->getName());
		lv_item->setData(1, Qt::UserRole, -1);            // -1 means not segment data
		m_lv_tree_items.append(lv_item);

		insertSegmentItems(lv, lv_item);
	    }
            if(lv->getName() == current_lv_name){
                current_item = lv_item;
            }        
        }
    }
    insertTopLevelItems(0, m_lv_tree_items);

    if( m_lv_tree_items.size() ){
        if( !current_item )
            setCurrentItem(m_lv_tree_items[0]);
        else{
            setCurrentItem(current_item); 	
            current_item->setExpanded(current_is_expanded);
        }
    }

    connect(this, SIGNAL(itemExpanded(QTreeWidgetItem *)), 
	    this, SLOT(adjustColumnWidth(QTreeWidgetItem *)));

    connect(this, SIGNAL(itemCollapsed(QTreeWidgetItem *)), 
	    this, SLOT(adjustColumnWidth(QTreeWidgetItem *)));

    setHiddenColumns();
    resizeColumnToContents(0);
    return;
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

void VGTree::insertSegmentItems(LogVol *logicalVolume, QTreeWidgetItem *item)
{

    QStringList segment_data;
    QTreeWidgetItem *lv_seg_item;

    for(int x = 0; x < logicalVolume->getSegmentCount(); x++){
	segment_data.clear();
	
	segment_data << QString("Seg# %1").arg(x) 
		     << sizeToString(logicalVolume->getSegmentSize(x)) 
		     << "" << "" << "" 
		     << QString("%1").arg(logicalVolume->getSegmentStripes(x))
		     << sizeToString(logicalVolume->getSegmentStripeSize(x)) 
		     << "" << "" << "" << "" ;
	
	lv_seg_item = new QTreeWidgetItem( item, segment_data );
	lv_seg_item->setData(0, Qt::UserRole, logicalVolume->getName());
	lv_seg_item->setData(1, Qt::UserRole, x);
    }
}

/* Here we start with the mirror logical volume and locate its mirror
   legs. Then the legs are checked for segment data just like any other
   logical volume and it is all inserted into the tree. The mirror log
   is also inserted here */

void VGTree::insertMirrorLegItems(LogVol *mirrorVolume, QTreeWidgetItem *item)
{
    QStringList leg_data;
    QTreeWidgetItem *leg_item;

    LogVol *leg_volume;

    VolGroup *volume_group = mirrorVolume->getVolumeGroup();    
    int lv_count = volume_group->getLogVolCount();
    QList<LogVol *>  logical_volume_list = volume_group->getLogicalVolumes();

    for(int x = 0; x < lv_count; x++){
	
	leg_volume = logical_volume_list[x];
	
	if( ( leg_volume->getOrigin() == mirrorVolume->getName() ) && 
	    ( leg_volume->isMirrorLog() || 
	      leg_volume->isMirrorLeg() ||
	      leg_volume->isVirtual() ||
	      leg_volume->isMirror() ) ){

	    leg_data.clear();

	    if( leg_volume->getSegmentCount() == 1 ) {	    

		leg_data << leg_volume->getName() 
			 << sizeToString(leg_volume->getSize()) << "" 
			 << leg_volume->getFilesystem()
			 << leg_volume->getType();
 
                if( !leg_volume->isMirror() ){
                    leg_data << QString("%1").arg( leg_volume->getSegmentStripes(0) ) 
                             << sizeToString( leg_volume->getSegmentStripeSize(0) );
                }
                else
                    leg_data << "" << "";


		if( leg_volume->isPvmove() )
		    leg_data    << QString("%%1").arg(leg_volume->getCopyPercent());
		else
		    leg_data << " ";
		
		leg_data << leg_volume->getState();
		
		if(leg_volume->isWritable())
		    leg_data << "r/w";
		else
		    leg_data << "r/o";
	    
		leg_item = new QTreeWidgetItem( item, leg_data );
		leg_item->setData(0, Qt::UserRole, leg_volume->getName());

// In the following "setData()" 0 means segment 0 (the only segment) 
// data is present on the same line as the rest of the lv data.

		leg_item->setData(1, Qt::UserRole, 0);  

		if( leg_volume->isMirror() )
                    insertMirrorLegItems(leg_volume, leg_item);    
            }
	    else {

		leg_data << leg_volume->getName() 
			 << sizeToString(leg_volume->getSize()) << "" 
			 << leg_volume->getFilesystem()
			 << leg_volume->getType() 
			 << "" << "";
		
		if( leg_volume->isPvmove() )
		    leg_data    << QString("%%1").arg(leg_volume->getCopyPercent());
		else
		    leg_data << " ";
		
		leg_data << leg_volume->getState();
		
		if(leg_volume->isWritable())
		    leg_data << "r/w";
		else
		    leg_data << "r/o";

		leg_item = new QTreeWidgetItem( item, leg_data );
		leg_item->setData(0, Qt::UserRole, leg_volume->getName());

// -1 means this item has no segment data. The segment data will be on 
// the following lines, one line per segement

		leg_item->setData(1, Qt::UserRole, -1);        
		insertSegmentItems(leg_volume, leg_item);
	    }
	}
    }
}

void VGTree::popupContextMenu(QPoint point)
{
    QTreeWidgetItem *item;
    KMenu *context_menu;

    item = itemAt(point);
    if(item){                                 //item = 0 if there is no item a that point
	m_lv_name = QVariant(item->data(0, Qt::UserRole)).toString();
	m_pv_name = QVariant(item->data(10,0)).toString();
	m_lv = m_vg->getLogVolByName(m_lv_name);

	context_menu = new LVActionsMenu(m_lv, m_vg, this);
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
