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

#include "deviceactionsmenu.h"
#include "devicetreeview.h"
#include "mkfs.h"
#include "mount.h"
#include "unmount.h"
#include "processprogress.h"
#include "pvcreate.h"
#include "pvremove.h"
#include "partremove.h"
#include "partadd.h"
#include "partmoveresize.h"
#include "vgreduce.h"
#include "vgreduceone.h"
#include "vgcreate.h"
#include "vgextend.h"
#include "topwindow.h"
#include "tablecreate.h"

extern MasterList *master_list;

DeviceTreeView::DeviceTreeView(QWidget *parent) : QTreeView(parent)
{

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), 
	    this, SLOT(popupContextMenu(QPoint)) );

}

void DeviceTreeView::popupContextMenu(QPoint point)
{

    KMenu *context_menu;

    index = indexAt(point);
    item = static_cast<StorageDeviceItem*> (index.internalPointer());

    if( (item->dataAlternate(0)).canConvert<void *>() )
        part = (StoragePartition *) (( item->dataAlternate(0)).value<void *>() );

    //item = 0 if there is no item a that point

    if(item){                          
        context_menu = new DeviceActionsMenu(item, this, this);
        context_menu->exec(QCursor::pos());
    }
    else{
        context_menu = new DeviceActionsMenu(NULL, this, this);
        context_menu->exec(QCursor::pos());
    }

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

void DeviceTreeView::addPartition()
{
  if( add_partition( part ) )
	MainWindow->reRun();
}

void DeviceTreeView::moveresizePartition()
{
  if( moveresize_partition( part ) )
	MainWindow->reRun();
}

void DeviceTreeView::vgcreatePartition()
{
    if( create_vg( item->data(0).toString() ) )
        MainWindow->reRun();
}

void DeviceTreeView::tablecreatePartition()
{
  if( create_table( item->data(0).toString() ) )
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

