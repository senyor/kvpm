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
#include "masterlist.h"
#include "processprogress.h"
#include "pvtree.h"
#include "pvmove.h"
#include "pvchange.h"
#include "physvol.h"
#include "sizetostring.h"
#include "topwindow.h"
#include "vgreduce.h"
#include "vgreduceone.h"
#include "volgroup.h"

extern MasterList *master_list;


PVTree::PVTree(QList<PhysVol *> physical_volumes, QWidget *parent) : QTreeWidget(parent)
{
    QList<PhysVol *>  pvs;
    PhysVol *pv;
    QStringList header_labels, pv_data;
    QTreeWidgetItem *item;

    pvs = physical_volumes;
    setupContextMenu();
    setColumnCount(6);
    header_labels << "Name" << "Size" << "Free" << "Used"
		  << "Allocatable" << "Exported";
    setHeaderLabels(header_labels);
    for(int x = 0; x < pvs.size(); x++){
	pv = pvs[x];
	pv_data.clear();
	    
	pv_data << pv->getDeviceName() 
		<< sizeToString(pv->getSize())
		<< sizeToString(pv->getUnused())
		<< sizeToString(pv->getUsed());
	if(pv->isAllocateable())
	    pv_data << "Yes";
	else
	    pv_data << "No";
	
	if(pv->isExported())
	    pv_data << "Yes";
	else
	    pv_data << "No";
	item = new QTreeWidgetItem((QTreeWidgetItem *)0, pv_data);
	item->setData(1, Qt::UserRole, pv->getSize());
	item->setData(2, Qt::UserRole, pv->getUnused());
	item->setData(3, Qt::UserRole, pv->getUsed());
	pv_tree_items.append(item);
    }
    insertTopLevelItems(0, pv_tree_items);
}

void PVTree::setupContextMenu()
{
    setContextMenuPolicy(Qt::CustomContextMenu);   
    pv_move_action = new QAction("Move Physical Extents", this);
    vg_reduce_action = new QAction("Remove from volume group", this);
    pv_change_action = new QAction("Change physical volume attributes", this);
    context_menu = new QMenu(this);
    context_menu->addAction(pv_move_action);
    context_menu->addAction(vg_reduce_action);
    context_menu->addAction(pv_change_action);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(popupContextMenu(QPoint)) );
    connect(pv_move_action, SIGNAL(triggered()), this, SLOT(movePhysicalExtents()));
    connect(vg_reduce_action, SIGNAL(triggered()), this, SLOT(reduceVolumeGroup()));
    connect(pv_change_action, SIGNAL(triggered()), this, SLOT(changePhysicalVolume()));
}

void PVTree::popupContextMenu(QPoint point)
{
    VolGroup *vg;
    QTreeWidgetItem *item;

    item = itemAt(point);
    if(item){
	if( (QVariant(item->data(3, 0)).toString()) == "0" ){  // 0 =  Zero used extents on pv
	    pv_name = QVariant(item->data(0, 0)).toString();
	    pv = ( master_list->getPhysVolByName(pv_name) );
	    vg_name = pv->getVolumeGroupName();
	    vg = ( master_list->getVolGroupByName(vg_name));
	    
	    pv_move_action->setEnabled(FALSE);
	    if(vg->getPhysVolCount() > 1)
		vg_reduce_action->setEnabled(TRUE);
	    else
		vg_reduce_action->setEnabled(FALSE);  // can't remove last pv from group
	}
	else{
	    pv_name = QVariant(item->data(0, 0)).toString();
	    pv = ( master_list->getPhysVolByName(pv_name) );
	    vg_name = pv->getVolumeGroupName();
	    vg = ( master_list->getVolGroupByName(vg_name));
	    
	    vg_reduce_action->setEnabled(FALSE);
	    if(vg->getPhysVolCount() > 1)              // can't move extents if there isn't another
		pv_move_action->setEnabled(TRUE);      // volume to put them on
	    else
		pv_move_action->setEnabled(FALSE);
	}
	context_menu->setEnabled(TRUE);
	context_menu->exec(QCursor::pos());
    }
    else
	context_menu->setEnabled(FALSE);  //item = 0 if there is no item a that point
}

void PVTree::movePhysicalExtents()
{
    PhysVol *pv = master_list->getPhysVolByName(pv_name);
    VolGroup *vg = master_list->getVolGroupByName(vg_name);

    if( move_pv(pv) )
	MainWindow->rebuildVolumeGroupTab(vg);
}

void PVTree::reduceVolumeGroup()
{
    if( reduce_vg_one(vg_name, pv_name) )
        MainWindow->reRun();
}

void PVTree::changePhysicalVolume()
{
    VolGroup *vg = master_list->getVolGroupByName(vg_name);

    PVChangeDialog dialog(pv_name);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress change_pv(dialog.arguments(), "Changing physical volume...");
	MainWindow->rebuildVolumeGroupTab(vg);
    }
}
