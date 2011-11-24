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


#include "pvtree.h"

#include <KConfigSkeleton>
#include <KIcon>
#include <KLocale>

#include <QtGui>

#include "logvol.h"
#include "masterlist.h"
#include "misc.h"
#include "processprogress.h"
#include "pvmove.h"
#include "pvchange.h"
#include "physvol.h"
#include "topwindow.h"
#include "vgreduce.h"
#include "vgreduceone.h"
#include "volgroup.h"


/* This is the physical volume tree list on the volume group tab */

PVTree::PVTree(VolGroup *volumeGroup, QWidget *parent) : QTreeWidget(parent), m_vg(volumeGroup)
{
    QStringList header_labels;
    m_context_menu = NULL;
    setColumnCount(6);
    QTreeWidgetItem * item;

    header_labels << i18nc("The name of the device", "Name") << i18n("Size") 
		  << i18nc("Unused space", "Remaining") << i18nc("Space used up", "Used")
		  << i18n("State") << i18n("Allocatable") 
                  << i18n("Tags") 
		  << i18n("Logical volumes");

    item = new QTreeWidgetItem((QTreeWidgetItem *)0, header_labels);

    for(int column = 0; column < 7; column++)
        item->setTextAlignment(column, Qt::AlignCenter);

    sortByColumn(0, Qt::AscendingOrder);

    item->setToolTip(0, i18n("Physical volume device"));
    item->setToolTip(1, i18n("Total size of physical volume"));
    item->setToolTip(2, i18n("Free space on physical volume"));
    item->setToolTip(3, i18n("Space used on physical volume"));
    item->setToolTip(4, i18n("A physcial volume is active if it has logical volumes that are active"));
    item->setToolTip(5, i18n("If physical volume allows more extents to be allocated"));
    item->setToolTip(6, i18n("Optional tags for physical volume"));
    item->setToolTip(7, i18n("Logical volumes on physical volume"));

    setHeaderItem(item);
}

void PVTree::loadData()
{
    QList<QTreeWidgetItem *> pv_tree_items;
    QList<LogVol *>  lvs = m_vg->getLogicalVolumesFlat();
    QList<PhysVol *> pvs = m_vg->getPhysicalVolumes();

    QStringList lv_name_list;
    QStringList pv_name_list;
    QString device_name;
    
    PhysVol *pv;
    QStringList pv_data;
    QTreeWidgetItem *item;
    
    QString old_current_pv_name;

    if( currentItem() )
        old_current_pv_name = currentItem()->data(0, 0).toString();

    clear();
    setupContextMenu();
    setSortingEnabled(false);
    
    for(int n = 0; n < pvs.size(); n++){
	pv = pvs[n];
	pv_data.clear();
	device_name = pv->getName();
	
	pv_data << device_name
		<< sizeToString( pv->getSize() )
		<< sizeToString( pv->getRemaining() )
		<< sizeToString( pv->getSize() - pv->getRemaining() );

	if( pv->isActive() )
	    pv_data << "Active";
	else
	    pv_data << "Inactive";

	if( pv->isAllocatable() )
	    pv_data << "Yes";
	else
	    pv_data << "No";

        pv_data << pv->getTags().join(", ");

/* here we get the names of logical volumes associated
   with the physical volume */

	lv_name_list.clear();
	for(int x = 0; x < lvs.size() ; x++){
	    pv_name_list = lvs[x]->getPvNamesAll();
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

        if( device_name == "unknown device" )
            item->setIcon(0, KIcon("exclamation"));
        else
            item->setIcon(0, KIcon());

	item->setData(0, Qt::UserRole, pv->getUuid());
	item->setData(1, Qt::UserRole, pv->getSize());
	item->setData(2, Qt::UserRole, pv->getRemaining());
	item->setData(3, Qt::UserRole, (pv->getSize() - pv->getRemaining()));

	if( pv->isActive() ){
            item->setToolTip(4, i18n("Active"));
            item->setIcon(4, KIcon("lightbulb"));
        }
	else{
            item->setToolTip(4, i18n("Inactive"));
            item->setIcon(4, KIcon("lightbulb_off"));
        }

        for(int column = 1; column < 6; column++)
            item->setTextAlignment(column, Qt::AlignRight);

        item->setTextAlignment(6, Qt::AlignLeft);
        item->setTextAlignment(7, Qt::AlignLeft);

	pv_tree_items.append(item);
    }
    insertTopLevelItems(0, pv_tree_items);


    setSortingEnabled(true);
    setHiddenColumns();

    if( !pv_tree_items.isEmpty() && !old_current_pv_name.isEmpty() ){
        bool match = false;
        for(int x = pv_tree_items.size() - 1; x >= 0; x--){ 
            if( old_current_pv_name == pv_tree_items[x]->data(0, 0).toString() ){ 
                setCurrentItem( pv_tree_items[x] );
                match = true;
                break;
            }
        }
        if(!match){
            setCurrentItem( pv_tree_items[0] );
            scrollToItem(pv_tree_items[0], QAbstractItemView::EnsureVisible);
        }
    }
    else{
        setCurrentItem( pv_tree_items[0] );
        scrollToItem(pv_tree_items[0], QAbstractItemView::EnsureVisible);
    }

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
    QStringList lvs;

    if(item){
	if( (QVariant(item->data(3, 0)).toString()) == "0" ){  // 0 =  Zero used extents on pv
	    m_pv_name = QVariant(item->data(0, 0)).toString();

            pv_move_action->setEnabled(false);
            
            if(m_vg->getPVCount() > 1)
                vg_reduce_action->setEnabled(true);
            else
                vg_reduce_action->setEnabled(false);  // can't remove last pv from group
        }
	else{
	    m_pv_name = QVariant(item->data(0, 0)).toString();
	    vg_reduce_action->setEnabled(false);

	    if(m_vg->getPVCount() > 1){     // can't move extents if there isn't another volume to put them on
                if( QVariant(item->data(6, 0)).toString().contains("pvmove") ) // can't have more than one pvmove
                    pv_move_action->setEnabled(false);                      // See physvol.cpp about removing this
                else
                    pv_move_action->setEnabled(true); 
            }
	    else
		pv_move_action->setEnabled(false);
	}
	m_context_menu->setEnabled(true);
	m_context_menu->exec(QCursor::pos());
    }
    else
	m_context_menu->setEnabled(false);  // item = 0 if there is no item a that point
}

void PVTree::movePhysicalExtents()
{
    PhysVol *pv = m_vg->getPvByName(m_pv_name);

    if(pv){
        PVMoveDialog dialog(pv);

        if( !dialog.bailout() )
            dialog.exec();
        
        if(dialog.result() == QDialog::Accepted)
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
    PhysVol *pv = m_vg->getPvByName(m_pv_name);
    if(pv){
        PVChangeDialog dialog(pv);
        dialog.exec();

        if(dialog.result() == QDialog::Accepted){
            ProcessProgress change_pv( dialog.arguments() );
            MainWindow->reRun();
        }
    }
}

void PVTree::setHiddenColumns()
{
    KConfigSkeleton skeleton;

    bool changed = false;

    bool pvname, pvsize,  pvremaining,
         pvused, pvstate, pvallocate,
         pvtags, pvlvnames;

    skeleton.setCurrentGroup("PhysicalTreeColumns");
    skeleton.addItemBool( "pvname",     pvname );
    skeleton.addItemBool( "pvsize",     pvsize );
    skeleton.addItemBool( "pvremaining",     pvremaining );
    skeleton.addItemBool( "pvused",     pvused );
    skeleton.addItemBool( "pvstate",    pvstate );
    skeleton.addItemBool( "pvallocate", pvallocate );
    skeleton.addItemBool( "pvtags",     pvtags );
    skeleton.addItemBool( "pvlvnames",  pvlvnames );

    if( !( !pvname == isColumnHidden(0)      && !pvsize == isColumnHidden(1) && 
           !pvremaining == isColumnHidden(2) && !pvused == isColumnHidden(3) &&
           !pvstate == isColumnHidden(4)     && !pvallocate == isColumnHidden(5) &&
           !pvtags == isColumnHidden(6)      && !pvlvnames == isColumnHidden(7) ) )
        changed = true;

    if(changed){
        setColumnHidden( 0, !pvname );
        setColumnHidden( 1, !pvsize );
        setColumnHidden( 2, !pvremaining );
        setColumnHidden( 3, !pvused );
        setColumnHidden( 4, !pvstate );
        setColumnHidden( 5, !pvallocate );
        setColumnHidden( 6, !pvtags );
        setColumnHidden( 7, !pvlvnames );

        for(int column = 0; column < 7; column++)
            if( !isColumnHidden(column) )
                resizeColumnToContents(column);
    }
}

