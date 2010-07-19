/*
 *
 * 
 * Copyright (C) 2008, 2010 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <KLocale>
#include <QtGui>

#include "logvol.h"
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

/* This is the physical volume tree list on the volume group tab */

PVTree::PVTree(VolGroup *volumeGroup, QWidget *parent) : QTreeWidget(parent), m_vg(volumeGroup)
{
    QStringList header_labels;

    setupContextMenu();
    setColumnCount(6);

    header_labels << i18n("Name") << i18n("Size") 
		  << i18n("Free") << i18n("Used")
		  << i18n("Allocatable") << i18n("Exported") 
		  << i18n("Logical volumes");

    setHeaderLabels(header_labels);
}

void PVTree::loadData()
{

    QList<QTreeWidgetItem *> pv_tree_items;
    QList<LogVol *>  lvs = m_vg->getLogicalVolumes();
    QList<PhysVol *> pvs = m_vg->getPhysicalVolumes();

    QStringList lv_name_list;
    QStringList pv_name_list;
    QString device_name;
    
    PhysVol *pv;
    QStringList pv_data;
    QTreeWidgetItem *item;

    clear();
    
    for(int n = 0; n < pvs.size(); n++){
	pv = pvs[n];
	pv_data.clear();
	device_name = pv->getDeviceName();
	
	pv_data << device_name
		<< sizeToString(pv->getSize())
		<< sizeToString(pv->getUnused())
		<< sizeToString(pv->getSize() - pv->getUnused());

	if(pv->isAllocateable())
	    pv_data << "Yes";
	else
	    pv_data << "No";
	
	if(pv->isExported())
	    pv_data << "Yes";
	else
	    pv_data << "No";

/* here we get the names of logical volumes associated
   with the physical volume */

	lv_name_list.clear();
	for(int x = 0; x < lvs.size() ; x++){
	    pv_name_list = lvs[x]->getDevicePathAll();
	    for(int y = 0; y < pv_name_list.size() ; y++)
		if( device_name == pv_name_list[y] ) 
		    lv_name_list.append( lvs[x]->getName() );
	}

/* next we remove duplicate entries */

	lv_name_list.sort();
	if( lv_name_list.size() > 1 )        
	    for(int x = lv_name_list.size() - 2; x >= 0; x--)
		if( lv_name_list[x] == lv_name_list[x + 1] )
		    lv_name_list.removeAt(x + 1);
	
	pv_data << lv_name_list.join(", ");

	item = new QTreeWidgetItem((QTreeWidgetItem *)0, pv_data);
	item->setData(1, Qt::UserRole, pv->getSize());
	item->setData(2, Qt::UserRole, pv->getUnused());
	item->setData(3, Qt::UserRole, (pv->getSize() - pv->getUnused()));
	pv_tree_items.append(item);
    }
    insertTopLevelItems(0, pv_tree_items);

    if( pv_tree_items.size() )
	setCurrentItem( pv_tree_items[0] );

    setColumnHidden(6, true);
    return;
}

void PVTree::setupContextMenu()
{
    setContextMenuPolicy(Qt::CustomContextMenu);   

    pv_move_action   = new QAction( i18n("Move physical extents"), this);
    vg_reduce_action = new QAction( i18n("Remove from volume group"), this);
    pv_change_action = new QAction( i18n("Change physical volume attributes"), this);

    context_menu = new QMenu(this);
    context_menu->addAction(pv_move_action);
    context_menu->addAction(vg_reduce_action);
    context_menu->addAction(pv_change_action);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), 
	    this, SLOT(popupContextMenu(QPoint)) );

    connect(pv_move_action, SIGNAL(triggered()), 
	    this, SLOT(movePhysicalExtents()));

    connect(vg_reduce_action, SIGNAL(triggered()), 
	    this, SLOT(reduceVolumeGroup()));

    connect(pv_change_action, SIGNAL(triggered()), 
	    this, SLOT(changePhysicalVolume()));
}

void PVTree::popupContextMenu(QPoint point)
{
    QTreeWidgetItem *item = itemAt(point);

    if(item){
	if( (QVariant(item->data(3, 0)).toString()) == "0" ){  // 0 =  Zero used extents on pv
	    m_pv_name = QVariant(item->data(0, 0)).toString();

            pv_move_action->setEnabled(false);
            
            if(m_vg->getPhysVolCount() > 1)
                vg_reduce_action->setEnabled(true);
            else
                vg_reduce_action->setEnabled(false);  // can't remove last pv from group
        }
	else{
	    m_pv_name = QVariant(item->data(0, 0)).toString();

	    vg_reduce_action->setEnabled(false);

	    if(m_vg->getPhysVolCount() > 1)            // can't move extents if there isn't
		pv_move_action->setEnabled(true);      // another volume to put them on
	    else
		pv_move_action->setEnabled(false);
	}
	context_menu->setEnabled(true);
	context_menu->exec(QCursor::pos());
    }
    else
	context_menu->setEnabled(false);  //item = 0 if there is no item a that point
}

void PVTree::movePhysicalExtents()
{
    PhysVol *pv = m_vg->getPhysVolByName(m_pv_name);
    if(pv){
        if( move_pv(pv) )
            MainWindow->reRun();
    }
}

void PVTree::reduceVolumeGroup()
{
    if( reduce_vg_one(m_vg->getName(), m_pv_name) )
        MainWindow->reRun();
}

void PVTree::changePhysicalVolume()
{
    PVChangeDialog dialog(m_pv_name);
    dialog.exec();

    if(dialog.result() == QDialog::Accepted){
        ProcessProgress change_pv(dialog.arguments(), i18n("Changing physical volume...") );
	MainWindow->reRun();
    }
}
