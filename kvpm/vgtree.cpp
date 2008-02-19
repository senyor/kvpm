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
#include "addmirror.h"
#include "lvactionsmenu.h"
#include "lvchange.h"
#include "lvcreate.h"
#include "lvreduce.h"
#include "lvremove.h"
#include "logvol.h"
#include "masterlist.h"
#include "mkfs.h"
#include "mount.h"
#include "processprogress.h"
#include "pvmove.h"
#include "removemirror.h"
#include "sizetostring.h"
#include "topwindow.h"
#include "unmount.h"
#include "vgtree.h"
#include "volgroup.h"

extern MasterList *master_list;



VGTree::VGTree(VolGroup *VolumeGroup) : QTreeWidget(), vg(VolumeGroup)
{
    QList<QTreeWidgetItem *> lv_tree_items;
    QList<LogVol *>  logical_volumes;
    QTreeWidgetItem *lv_item, *lv_seg_item;
    QStringList header_labels, lv_data;

    vg_name = vg->getName();
    setupContextMenu();
    
    setColumnCount(9);
    header_labels << "Name" << "Size" << "Filesystem" << "type"
		  << "Stripes" << "Stripe size" 
		  << "Snap/Move" << "State" << "Access" ;
    
    setHeaderLabels(header_labels);
    logical_volumes = vg->getLogicalVolumes();

    for(int x = 0; x < vg->getLogVolCount(); x++){
	lv = logical_volumes[x];
	lv_data.clear();
	if(lv->getSegmentCount() == 1){
	    lv_data << lv->getName() << sizeToString(lv->getSize()) << lv->getFilesystem()
		    << lv->getType() << QString("%1").arg(lv->getSegmentStripes(0)) 
		    << sizeToString(lv->getSegmentStripeSize(0));

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
	    lv_item = new QTreeWidgetItem((QTreeWidgetItem *)0, lv_data);
	    lv_tree_items.append(lv_item);
	    lv_item->setData(0, Qt::UserRole, lv->getName());
	    lv_item->setData(1, Qt::UserRole, 0);            // 0 means segment 0 data present
	}
	else {
	    lv_data << lv->getName() << sizeToString(lv->getSize()) << lv->getFilesystem()
		    << lv->getType() << "" << "";

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
	    lv_item = new QTreeWidgetItem((QTreeWidgetItem *)0, lv_data);
	    lv_item->setData(0, Qt::UserRole, lv->getName());
	    lv_item->setData(1, Qt::UserRole, -1);            // -1 means not segment data
	    lv_tree_items.append(lv_item);
	    for(int x = 0; x < lv->getSegmentCount(); x++){
		lv_data.clear();
		lv_data << QString("Seg# %1").arg(x + 1) 
			<< sizeToString(lv->getSegmentSize(x)) << "" << "" 
			<< QString("%1").arg(lv->getSegmentStripes(x))
			<< sizeToString(lv->getSegmentStripeSize(x)) << "" << "";
		lv_data << "" << "" ;
		lv_seg_item = new QTreeWidgetItem(lv_item, lv_data);
		lv_seg_item->setData(0, Qt::UserRole, lv->getName());
		lv_seg_item->setData(1, Qt::UserRole, x);            // x == seg number
	    }
	}
    }
    insertTopLevelItems(0, lv_tree_items);
}

void VGTree::setupContextMenu()
{
    setContextMenuPolicy(Qt::CustomContextMenu);   

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), 
	    this, SLOT(popupContextMenu(QPoint)) );

}

void VGTree::popupContextMenu(QPoint point)
{
    QTreeWidgetItem *item;
    KMenu *context_menu;
    
    item = itemAt(point);
    if(item){                                 //item = 0 if there is no item a that point
	lv_name = QVariant(item->data(0, Qt::UserRole)).toString();
	pv_name = QVariant(item->data(10,0)).toString();
	lv = vg->getLogVolByName(lv_name);

	context_menu = new LVActionsMenu(lv, this, this);
	context_menu->exec(QCursor::pos());
    }
    else{
	context_menu = new LVActionsMenu(NULL, this, this);
	context_menu->exec(QCursor::pos());
    }
	    
}

void VGTree::mkfsLogicalVolume()
{
    if( make_fs(lv) )
	MainWindow->rebuildVolumeGroupTab();
}

void VGTree::removeLogicalVolume()
{
    if( remove_lv(vg_name + "/" + lv_name) )
	MainWindow->reRun();
}

void VGTree::createLogicalVolume()
{
    if(LVCreate(vg))
	MainWindow->reRun();
}

void VGTree::createSnapshot()
{
    if(SnapshotCreate(vg->getLogVolByName(lv_name)))
	MainWindow->reRun();
}

void VGTree::extendLogicalVolume()
{
    LogVol *lv = vg->getLogVolByName(lv_name);

    if(LVExtend(lv))
	MainWindow->reRun();
}


void VGTree::reduceLogicalVolume()
{
    LogVol *lv = vg->getLogVolByName(lv_name);

    if(LVReduce(lv))
	MainWindow->reRun();
}

void VGTree::movePhysicalExtents()
{
    LogVol *lv = vg->getLogVolByName(lv_name);

    if( move_pv(lv) )
	MainWindow->reRun();
}

void VGTree::changeLogicalVolume()
{
    LogVol *lv = vg->getLogVolByName(lv_name);

    if( change_lv(lv) )
	MainWindow->rebuildVolumeGroupTab();
}

void VGTree::addMirror()
{
    LogVol *lv = vg->getLogVolByName(lv_name);

    if( add_mirror(lv) )
	MainWindow->reRun();
}

void VGTree::removeMirror()
{
    LogVol *lv = vg->getLogVolByName(lv_name);

    if( remove_mirror(lv) )
	MainWindow->reRun();
}

void VGTree::mountFilesystem()
{
    LogVol *lv = vg->getLogVolByName(lv_name);

    if( mount_filesystem(lv) )
	MainWindow->rebuildVolumeGroupTab();
}

void VGTree::unmountFilesystem()
{
    LogVol *lv = vg->getLogVolByName(lv_name);

    if( unmount_filesystem(lv) ) 
	MainWindow->rebuildVolumeGroupTab();
}
