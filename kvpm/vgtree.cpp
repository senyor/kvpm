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



VGTree::VGTree(VolGroup *VolumeGroup) : QTreeWidget(),
					m_vg(VolumeGroup)
{
    QList<QTreeWidgetItem *> lv_tree_items;
    QList<LogVol *>  logical_volumes;

    QTreeWidgetItem *lv_item;
    QTreeWidgetItem *lv_seg_item;

    QStringList header_labels;
    QStringList lv_data;

    m_vg_name = m_vg->getName();
    setupContextMenu();
    
    setColumnCount(9);
    header_labels << "Name" << "Size" << "Filesystem" << "type"
		  << "Stripes" << "Stripe size" 
		  << "Snap/Move" << "State" << "Access" ;
    setHeaderLabels(header_labels);

    logical_volumes = m_vg->getLogicalVolumes();

    for(int x = 0; x < m_vg->getLogVolCount(); x++){
	m_lv = logical_volumes[x];
	lv_data.clear();
	if(m_lv->getSegmentCount() == 1){
	    lv_data << m_lv->getName() << sizeToString(m_lv->getSize()) << m_lv->getFilesystem()
		    << m_lv->getType() << QString("%1").arg(m_lv->getSegmentStripes(0)) 
		    << sizeToString(m_lv->getSegmentStripeSize(0));

	    if( m_lv->isSnap() )
		lv_data    << QString("%%1").arg(m_lv->getSnapPercent());
	    else if( m_lv->isPvmove() )
		lv_data    << QString("%%1").arg(m_lv->getCopyPercent());
	    else
		lv_data << " ";
	    
	    lv_data << m_lv->getState();
	    if(m_lv->isWritable())
		lv_data << "r/w";
	    else
		lv_data << "r/o";
	    lv_item = new QTreeWidgetItem((QTreeWidgetItem *)0, lv_data);
	    lv_tree_items.append(lv_item);
	    lv_item->setData(0, Qt::UserRole, m_lv->getName());
	    lv_item->setData(1, Qt::UserRole, 0);            // 0 means segment 0 data present
	}
	else {
	    lv_data << m_lv->getName() 
		    << sizeToString(m_lv->getSize()) 
		    << m_lv->getFilesystem()
		    << m_lv->getType() 
		    << "" << "";

	    if( m_lv->isSnap() )
		lv_data    << QString("%%1").arg(m_lv->getSnapPercent());
	    else if( m_lv->isPvmove() )
		lv_data    << QString("%%1").arg(m_lv->getCopyPercent());
	    else
		lv_data << " ";
	    
	    lv_data << m_lv->getState();

	    if(m_lv->isWritable())
		lv_data << "r/w";
	    else
		lv_data << "r/o";

	    lv_item = new QTreeWidgetItem((QTreeWidgetItem *)0, lv_data);
	    lv_item->setData(0, Qt::UserRole, m_lv->getName());
	    lv_item->setData(1, Qt::UserRole, -1);            // -1 means not segment data
	    lv_tree_items.append(lv_item);
	    for(int x = 0; x < m_lv->getSegmentCount(); x++){
		lv_data.clear();
		lv_data << QString("Seg# %1").arg(x + 1) 
			<< sizeToString(m_lv->getSegmentSize(x)) 
			<< "" << "" 
			<< QString("%1").arg(m_lv->getSegmentStripes(x))
			<< sizeToString(m_lv->getSegmentStripeSize(x)) 
			<< "" << "";
		lv_data << "" << "" ;
		lv_seg_item = new QTreeWidgetItem(lv_item, lv_data);
		lv_seg_item->setData(0, Qt::UserRole, m_lv->getName());
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
	m_lv_name = QVariant(item->data(0, Qt::UserRole)).toString();
	m_pv_name = QVariant(item->data(10,0)).toString();
	m_lv = m_vg->getLogVolByName(m_lv_name);

	context_menu = new LVActionsMenu(m_lv, this, this);
	context_menu->exec(QCursor::pos());
    }
    else{
	context_menu = new LVActionsMenu(NULL, this, this);
	context_menu->exec(QCursor::pos());
    }
}

void VGTree::mkfsLogicalVolume()
{
    if( make_fs(m_lv) )
	MainWindow->rebuildVolumeGroupTab();
}

void VGTree::removeLogicalVolume()
{
    if( remove_lv(m_vg_name + "/" + m_lv_name) )
	MainWindow->reRun();
}

void VGTree::createLogicalVolume()
{
    if( LVCreate(m_vg) )
	MainWindow->reRun();
}

void VGTree::createSnapshot()
{
    if( SnapshotCreate(m_lv) )
	MainWindow->reRun();
}

void VGTree::extendLogicalVolume()
{
    if( LVExtend(m_lv) )
	MainWindow->reRun();
}


void VGTree::reduceLogicalVolume()
{
    if( LVReduce(m_lv) )
	MainWindow->reRun();
}

void VGTree::movePhysicalExtents()
{
    if( move_pv(m_lv) )
	MainWindow->reRun();
}

void VGTree::changeLogicalVolume()
{
    if( change_lv(m_lv) )
	MainWindow->rebuildVolumeGroupTab();
}

void VGTree::addMirror()
{
    if( add_mirror(m_lv) )
	MainWindow->reRun();
}

void VGTree::removeMirror()
{
    if( remove_mirror(m_lv) )
	MainWindow->reRun();
}

void VGTree::mountFilesystem()
{
    if( mount_filesystem(m_lv) )
	MainWindow->rebuildVolumeGroupTab();
}

void VGTree::unmountFilesystem()
{
    if( unmount_filesystem(m_lv) ) 
	MainWindow->rebuildVolumeGroupTab();
}
