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
#include "devicesizechartseg.h"
#include "masterlist.h"
#include "mkfs.h"
#include "processprogress.h"
#include "pvcreate.h"
#include "pvremove.h"
#include "topwindow.h"
#include "vgcreate.h"
#include "vgextend.h"
#include "vgreduce.h"
#include "vgreduceone.h"

extern MasterList *master_list;

DeviceChartSeg::DeviceChartSeg(StorageDeviceItem *device_item, QWidget *parent) : QWidget(parent)
{
    QStringList group_names;
    
    item = device_item;

    part = NULL;
    if( (item->dataAlternate(0)).canConvert<void *>() )
	part = (StoragePartition *) (( item->dataAlternate(0)).value<void *>() );

    path = (item->data(0)).toString();
    use =  (item->data(4)).toString();
    QPalette *colorset = new QPalette();

    if(path == "freespace")
	colorset->setColor(QPalette::Window, Qt::green);
    else{
	if(use == "ext2")
	    colorset->setColor(QPalette::Window, Qt::blue);
	else if(use == "ext3")
	    colorset->setColor(QPalette::Window, Qt::darkBlue);
	else if(use == "reiserfs")
	    colorset->setColor(QPalette::Window, Qt::red);
	else if(use == "vfat")
	    colorset->setColor(QPalette::Window, Qt::yellow);
	else if(use == "jfs")
	    colorset->setColor(QPalette::Window, Qt::darkGray);
	else if(use == "xfs")
	    colorset->setColor(QPalette::Window, Qt::cyan);
	else if(use == "swap")
	    colorset->setColor(QPalette::Window, Qt::lightGray);
	else if(use == "physical volume")
	    colorset->setColor(QPalette::Window, Qt::magenta);
	else
	    colorset->setColor(QPalette::Window, Qt::black);
    }
	
    setPalette(*colorset);
    setAutoFillBackground(TRUE);

    setContextMenuPolicy(Qt::CustomContextMenu);
    menu = new QMenu(this);
    vgextend_menu = new QMenu("Extend Volume Group", this);

    mkfs_action     = new QAction("Make Filesystem", this);
    pvcreate_action = new QAction("Create physical volume", this);
    pvremove_action = new QAction("Remove physical volume", this);
    vgcreate_action = new QAction("Create volume group", this);
    vgreduce_action = new QAction("Remove from volume group", this);
    mount_action    = new QAction("Mount filesystem", this);
    unmount_action  = new QAction("Unmount filesystem", this);
    
    group_names = master_list->getVolumeGroupNames();
    
    for(int x = 0; x < group_names.size(); x++){
	vgextend_actions.append(new QAction(group_names[x], this));
	vgextend_menu->addAction(vgextend_actions[x]);
    }
    menu->addAction(mkfs_action);
    menu->addAction(pvcreate_action);
    menu->addAction(pvremove_action);
    menu->addAction(vgcreate_action);
    menu->addAction(vgreduce_action);
    menu->addAction(mount_action);
    menu->addAction(unmount_action);
    
    menu->addMenu(vgextend_menu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), 
	    this, SLOT(popupContextMenu(QPoint)) );

    connect(mkfs_action, SIGNAL(triggered()), this, SLOT(mkfsPartition()));
    connect(pvcreate_action, SIGNAL(triggered()), this, SLOT(pvcreatePartition()));
    connect(pvremove_action, SIGNAL(triggered()), this, SLOT(pvremovePartition()));
    connect(vgcreate_action, SIGNAL(triggered()), this, SLOT(vgcreatePartition()));
    connect(vgreduce_action, SIGNAL(triggered()), this, SLOT(vgreducePartition()));
    connect(vgextend_menu, SIGNAL(triggered(QAction*)), this, SLOT(vgextendPartition(QAction*)));
}

void DeviceChartSeg::popupContextMenu(QPoint point)
{
    (void)point;

    if(item->data(6) == "yes"){
	mount_action->setEnabled(FALSE);
	unmount_action->setEnabled(TRUE);
    }
    else if(item->data(6) == "no"){
	mount_action->setEnabled(TRUE);
	unmount_action->setEnabled(FALSE);
    }
    else{
	mount_action->setEnabled(FALSE);
	unmount_action->setEnabled(FALSE);
    }
    
    if(item->data(0) == "freespace" || item->data(1) == "extended" ){
	pvcreate_action->setEnabled(FALSE);
	mkfs_action->setEnabled(FALSE);
	pvremove_action->setEnabled(FALSE);
	vgcreate_action->setEnabled(FALSE);
	vgextend_menu->setEnabled(FALSE);
	vgreduce_action->setEnabled(FALSE);
    }
    else if(item->data(6) == "yes"){
	pvcreate_action->setEnabled(FALSE);
	mkfs_action->setEnabled(FALSE);
	pvremove_action->setEnabled(FALSE);
	vgcreate_action->setEnabled(FALSE);
	vgextend_menu->setEnabled(FALSE);
	vgreduce_action->setEnabled(FALSE);
    }
    else if( (item->data(4) == "physical volume") && (item->data(5) == "" ) ){
	pvcreate_action->setEnabled(FALSE);
	mkfs_action->setEnabled(FALSE);
	pvremove_action->setEnabled(TRUE);
	vgcreate_action->setEnabled(TRUE);
	vgextend_menu->setEnabled(TRUE);
	vgreduce_action->setEnabled(FALSE);
    }
    else if( (item->data(4) == "physical volume") && (item->data(5) != "" ) ){
	pvcreate_action->setEnabled(FALSE);
	mkfs_action->setEnabled(FALSE);
	pvremove_action->setEnabled(FALSE);
	vgcreate_action->setEnabled(FALSE);
	vgextend_menu->setEnabled(FALSE);
	if( item->dataAlternate(3) == 0 )
	    vgreduce_action->setEnabled(TRUE);
	else
	    vgreduce_action->setEnabled(FALSE);
    }
    else if(item->data(1) == "logical" || item->data(1) == "normal"){
	pvcreate_action->setEnabled(TRUE);
	pvremove_action->setEnabled(FALSE);
	mkfs_action->setEnabled(TRUE);
	vgcreate_action->setEnabled(FALSE);
	vgextend_menu->setEnabled(FALSE);
	vgreduce_action->setEnabled(FALSE);
    }
    else{
	pvcreate_action->setEnabled(FALSE);
	pvremove_action->setEnabled(FALSE);
	mkfs_action->setEnabled(FALSE);
	vgcreate_action->setEnabled(FALSE);
	vgextend_menu->setEnabled(FALSE);
	vgreduce_action->setEnabled(FALSE);
    }
    menu->exec(QCursor::pos());
    menu->setEnabled(TRUE);
}

void DeviceChartSeg::mkfsPartition()
{
    if( make_fs(part) )
        MainWindow->reRun();
}

void DeviceChartSeg::pvcreatePartition()
{
    if( create_pv(path) )
	MainWindow->reRun();
}

void DeviceChartSeg::pvremovePartition()
{
    if( remove_pv(path) )
	MainWindow->reRun();
}

void DeviceChartSeg::vgcreatePartition()
{
    if( create_vg(path) )
	MainWindow->reRun();
}

void DeviceChartSeg::vgreducePartition()
{
    if( reduce_vg_one( item->data(4).toString(), path) )
	MainWindow->reRun();
}

void DeviceChartSeg::vgextendPartition(QAction *action)
{
    QString group = action->text();
    group.remove(QChar('&'));

    if( extend_vg(group, path) )
	MainWindow->reRun();
}
