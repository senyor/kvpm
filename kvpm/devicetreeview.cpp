/*
 *
 * 
 * Copyright (C) 2008, 2009 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <QtGui>
#include <KLocale>

#include "devicetreeview.h"
#include "mkfs.h"
#include "mount.h"
#include "unmount.h"
#include "processprogress.h"
#include "pvcreate.h"
#include "pvremove.h"
#include "partremove.h"
#include "vgreduce.h"
#include "vgreduceone.h"
#include "vgcreate.h"
#include "vgextend.h"
#include "topwindow.h"

extern MasterList *master_list;

DeviceTreeView::DeviceTreeView(QWidget *parent) : QTreeView(parent)
{
    QStringList group_names;
 
    setContextMenuPolicy(Qt::CustomContextMenu);
    menu = new KMenu(this);
    vgextend_menu     = new KMenu( i18n("Extend volume group"), this);
    mkfs_action       = new KAction( i18n("Make filesystem"), this);
    partremove_action = new KAction( i18n("Remove disk partition"), this);
    pvcreate_action   = new KAction( i18n("Create physical volume"), this);
    pvremove_action   = new KAction( i18n("Remove physical volume"), this);
    vgcreate_action   = new KAction( i18n("Create volume group"), this);
    vgreduce_action   = new KAction( i18n("Remove from volume group"), this);
    mount_action      = new KAction( i18n("Mount filesystem"), this);
    unmount_action    = new KAction( i18n("Unmount filesystem"), this);
    menu->addAction(mkfs_action);
    menu->addAction(partremove_action);
    menu->addAction(pvcreate_action);
    menu->addAction(pvremove_action);
    menu->addAction(vgcreate_action);
    menu->addAction(vgreduce_action);
    menu->addMenu(vgextend_menu);
    menu->addAction(mount_action);
    menu->addAction(unmount_action);

    group_names = master_list->getVolumeGroupNames();
    for(int x = 0; x < group_names.size(); x++){
        vgextend_actions.append(new QAction(group_names[x], this));
        vgextend_menu->addAction(vgextend_actions[x]);
    }
    
    connect(mkfs_action,       SIGNAL(triggered()), this, SLOT(mkfsPartition()));
    connect(partremove_action, SIGNAL(triggered()), this, SLOT(removePartition()));
    connect(pvcreate_action,   SIGNAL(triggered()), this, SLOT(pvcreatePartition()));
    connect(pvremove_action,   SIGNAL(triggered()), this, SLOT(pvremovePartition()));
    connect(vgcreate_action,   SIGNAL(triggered()), this, SLOT(vgcreatePartition()));
    connect(vgreduce_action,   SIGNAL(triggered()), this, SLOT(vgreducePartition()));
    connect(mount_action,      SIGNAL(triggered()), this, SLOT(mountPartition()));
    connect(unmount_action,    SIGNAL(triggered()), this, SLOT(unmountPartition()));

    connect(vgextend_menu, SIGNAL(triggered(QAction*)), 
	    this, SLOT(vgextendPartition(QAction*)));

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), 
	    this, SLOT(popupContextMenu(QPoint)) );

}

void DeviceTreeView::popupContextMenu(QPoint point)
{
    index = indexAt(point);
    item = static_cast<StorageDeviceItem*> (index.internalPointer());
    if(item){

	if( (item->dataAlternate(0)).canConvert<void *>() )
	    part = (StoragePartition *) (( item->dataAlternate(0)).value<void *>() );

	menu->setEnabled(true);


	if(item->data(6) == "yes"){
	    mount_action->setEnabled(true);
	    unmount_action->setEnabled(true);
	}
	else if(item->data(6) == "no"){
	    mount_action->setEnabled(true);
	    unmount_action->setEnabled(false);
	}
	else{
	    mount_action->setEnabled(false);
	    unmount_action->setEnabled(false);
	}

	if(item->data(1) == "freespace" || 
	   item->data(1) == "freespace (logical)")
	  {
	    pvcreate_action->setEnabled(false);
	    mkfs_action->setEnabled(false);
	    partremove_action->setEnabled(false);
	    pvremove_action->setEnabled(false);
	    vgcreate_action->setEnabled(false);
	    vgextend_menu->setEnabled(false);
	    vgreduce_action->setEnabled(false);
	}
	else if(item->data(1) == "extended"){
	    pvcreate_action->setEnabled(false);
	    mkfs_action->setEnabled(false);
	    partremove_action->setEnabled(true);
	    pvremove_action->setEnabled(false);
	    vgcreate_action->setEnabled(false);
	    vgextend_menu->setEnabled(false);
	    vgreduce_action->setEnabled(false);
	}
	else if(item->data(6) == "yes"){
	    pvcreate_action->setEnabled(false);
	    partremove_action->setEnabled(false);
	    mkfs_action->setEnabled(false);
	    pvremove_action->setEnabled(false);
	    vgcreate_action->setEnabled(false);
	    vgextend_menu->setEnabled(false);
	    vgreduce_action->setEnabled(false);
	}
	else if( (item->data(4) == "physical volume") && (item->data(5) == "" ) ){
	    pvcreate_action->setEnabled(false);
	    mkfs_action->setEnabled(false);
	    partremove_action->setEnabled(false);
	    pvremove_action->setEnabled(true);
	    vgcreate_action->setEnabled(true);
	    vgextend_menu->setEnabled(true);
	    vgreduce_action->setEnabled(false);
	}
	else if( (item->data(4) == "physical volume") && (item->data(5) != "" ) ){
	    pvcreate_action->setEnabled(false);
	    mkfs_action->setEnabled(false);
	    partremove_action->setEnabled(false);
	    pvremove_action->setEnabled(false);
	    vgcreate_action->setEnabled(false);
	    vgextend_menu->setEnabled(false);
	    if( item->dataAlternate(3) == 0 )
		vgreduce_action->setEnabled(true);
	    else
		vgreduce_action->setEnabled(false);
	}
	else if(item->data(1) == "logical" || item->data(1) == "normal"){

	    if(item->data(6) == "no")
   	        partremove_action->setEnabled(true);
	    else
	        partremove_action->setEnabled(false);

	    pvcreate_action->setEnabled(true);
	    pvremove_action->setEnabled(false);
	    mkfs_action->setEnabled(true);
	    vgcreate_action->setEnabled(false);
	    vgextend_menu->setEnabled(false);
	    vgreduce_action->setEnabled(false);
	}
	else{
	    partremove_action->setEnabled(false);
	    pvcreate_action->setEnabled(false);
	    pvremove_action->setEnabled(false);
	    mkfs_action->setEnabled(false);
	    vgcreate_action->setEnabled(false);
	    vgextend_menu->setEnabled(false);
	    vgreduce_action->setEnabled(false);
	}
	menu->exec(QCursor::pos());
    }
    else
	menu->setEnabled(false);  // if item points to NULL, do nothing
}

void DeviceTreeView::mkfsPartition()
{
    if( make_fs(part) )
	    MainWindow->reRun();
}

void DeviceTreeView::pvcreatePartition()
{
    if( create_pv(item->data(0).toString() ) )
	MainWindow->reRun();
}

void DeviceTreeView::pvremovePartition()
{
    if( remove_pv( item->data(0).toString() ) )
	MainWindow->reRun();
}

void DeviceTreeView::removePartition()
{
  if( remove_partition( part ) )
	MainWindow->reRun();
}

void DeviceTreeView::vgcreatePartition()
{
    if( create_vg( item->data(0).toString() ) )
        MainWindow->reRun();
}

void DeviceTreeView::vgreducePartition()
{
    if( reduce_vg_one( item->data(5).toString(), item->data(0).toString() ) )
	MainWindow->reRun();
}

void DeviceTreeView::vgextendPartition(QAction *action)
{
    QString group = action->text();
    group.remove(QChar('&'));
    QString pv_path = item->data(0).toString();

    if( extend_vg(group, pv_path) )
	MainWindow->reRun();
}

void DeviceTreeView::mountPartition()
{
    if( mount_filesystem(part) )
	MainWindow->reRun();
}

void DeviceTreeView::unmountPartition()
{
    if( unmount_filesystem(part) )
	MainWindow->reRun();
}

