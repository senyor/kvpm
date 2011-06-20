/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
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
#include "misc.h"
#include "topwindow.h"
#include "vgreduce.h"
#include "vgreduceone.h"
#include "volgroup.h"

extern MasterList *master_list;

/* This is the physical volume tree list on the volume group tab */

PVTree::PVTree(VolGroup *volumeGroup, QWidget *parent) : QTreeWidget(parent), m_vg(volumeGroup)
{
    QStringList header_labels;
    m_context_menu = NULL;
    setColumnCount(6);
    QTreeWidgetItem * item;

    header_labels << i18n("Name") << i18n("Size") 
		  << i18n("Free") << i18n("Used")
		  << i18n("Allocatable") 
                  << i18n("MDA Count") << i18n("MDA Size") 
                  << i18n("Tags") 
		  << i18n("Logical volumes");

    item = new QTreeWidgetItem((QTreeWidgetItem *)0, header_labels);

    for(int column = 0; column < 8; column++)
        item->setTextAlignment(column, Qt::AlignCenter);

    item->setToolTip(0, i18n("Physical volume device"));
    item->setToolTip(1, i18n("Total size of physical volume"));
    item->setToolTip(2, i18n("Free space on physical volume"));
    item->setToolTip(3, i18n("Space used on physical volume"));
    item->setToolTip(4, i18n("If physical volume allows more extents to be allocated"));
    item->setToolTip(5, i18n("Number of metadata areas on physical volume"));
    item->setToolTip(6, i18n("Size of meta data areas on physical volume"));
    item->setToolTip(7, i18n("Optional tags for physical volume"));

    setHeaderItem(item);
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
    setupContextMenu();
    
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

        pv_data << QString("%1").arg(pv->getMDACount());
        pv_data << sizeToString(pv->getMDASize());

        pv_data << "   "; // replace with pv->getTag();

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

        for(int column = 1; column < 7; column++)
            item->setTextAlignment(column, Qt::AlignRight);

	pv_tree_items.append(item);
    }
    insertTopLevelItems(0, pv_tree_items);

    if( pv_tree_items.size() )
	setCurrentItem( pv_tree_items[0] );

    setColumnHidden(8, true);
    return;
}

void PVTree::setupContextMenu()
{
    setContextMenuPolicy(Qt::CustomContextMenu);   

    pv_move_action   = new QAction( i18n("Move physical extents"), this);
    vg_reduce_action = new QAction( i18n("Remove from volume group"), this);
    pv_change_action = new QAction( i18n("Change physical volume attributes"), this);

    if(m_context_menu)
        m_context_menu->deleteLater();
    m_context_menu = new QMenu(this);
    m_context_menu->addAction(pv_move_action);
    m_context_menu->addAction(vg_reduce_action);
    m_context_menu->addAction(pv_change_action);

    // disconnect the last run's connections or they pile up.

    disconnect(this, SIGNAL(customContextMenuRequested(QPoint)), 
	       this, SLOT(popupContextMenu(QPoint)) );

    disconnect(pv_move_action, SIGNAL(triggered()), 
	       this, SLOT(movePhysicalExtents()));

    disconnect(vg_reduce_action, SIGNAL(triggered()), 
	       this, SLOT(reduceVolumeGroup()));

    disconnect(pv_change_action, SIGNAL(triggered()), 
	       this, SLOT(changePhysicalVolume()));

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
	m_context_menu->setEnabled(true);
	m_context_menu->exec(QCursor::pos());
    }
    else
	m_context_menu->setEnabled(false);  //item = 0 if there is no item a that point
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