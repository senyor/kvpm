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

#include "devicemodel.h"
#include "devicesizechartseg.h"
#include "deviceactionsmenu.h"
#include "masterlist.h"
#include "mkfs.h"
#include "mount.h"
#include "partadd.h"
#include "partremove.h"
#include "processprogress.h"
#include "pvcreate.h"
#include "pvremove.h"
#include "storagepartition.h"
#include "topwindow.h"
#include "unmount.h"
#include "vgcreate.h"
#include "vgextend.h"
#include "vgreduce.h"
#include "vgreduceone.h"


extern MasterList *master_list;

DeviceChartSeg::DeviceChartSeg(StorageDeviceItem *storageDeviceItem, QWidget *parent) : 
    QFrame(parent),
    m_item(storageDeviceItem)
{
    QStringList group_names;
    QString use;

    if (( m_item->data(1)).toString()== "extended" ){
        setFrameStyle(QFrame::Raised | QFrame::Box);
        setLineWidth( 1 );
    }
    else if (( m_item->data(1)).toString()== "logical" ){
        setFrameStyle(QFrame::Sunken | QFrame::Panel);
        setLineWidth( 2 );
    }
    else if (( m_item->data(1)).toString()== "logical (freespace)" ){
        setFrameStyle(QFrame::Sunken | QFrame::Panel);
        setLineWidth( 2 );
    }
    else{
        setFrameStyle( QFrame::Sunken | QFrame::Panel );
	setLineWidth( 2 );
    }

    m_partition = NULL;
    if( (m_item->dataAlternate(0)).canConvert<void *>() )
	m_partition = (StoragePartition *) (( m_item->dataAlternate(0)).value<void *>() );

    m_pv_name = (m_item->data(1)).toString();

    use =  (m_item->data(4)).toString();
    QPalette *colorset = new QPalette();

    if(m_pv_name == "freespace" || m_pv_name == "freespace (logical)"){
	colorset->setColor(QPalette::Window, Qt::green);
    }
    else if( m_pv_name != "extended" ){

        if(use == "ext2")
	    colorset->setColor(QPalette::Window, Qt::blue);
	else if(use == "ext3")
	    colorset->setColor(QPalette::Window, Qt::darkBlue);
	else if(use == "ext4")
	    colorset->setColor(QPalette::Window, Qt::cyan);
	else if(use == "reiserfs")
	    colorset->setColor(QPalette::Window, Qt::red);
	else if(use == "reiser4")
	    colorset->setColor(QPalette::Window, Qt::darkRed);
	else if(use == "hfs")
	    colorset->setColor(QPalette::Window, Qt::magenta);
	else if(use == "vfat")
	    colorset->setColor(QPalette::Window, Qt::yellow);
	else if(use == "jfs")
	    colorset->setColor(QPalette::Window, Qt::darkMagenta);
	else if(use == "xfs")
	    colorset->setColor(QPalette::Window, Qt::darkCyan);
	else if(use == "swap")
	    colorset->setColor(QPalette::Window, Qt::lightGray);
	else if(use == "freespace")
	    colorset->setColor(QPalette::Window, Qt::green);
	else if(use == "physical volume")
	    colorset->setColor(QPalette::Window, Qt::darkGreen);
	else
	    colorset->setColor(QPalette::Window, Qt::black);
    }
	
    setPalette(*colorset);
    setAutoFillBackground(true);

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), 
	    this, SLOT(popupContextMenu(QPoint)) );

}

void DeviceChartSeg::popupContextMenu(QPoint point)
{
    (void)point;

    KMenu *context_menu;

    if( (m_item->dataAlternate(0)).canConvert<void *>() )
        m_partition = (StoragePartition *) (( m_item->dataAlternate(0)).value<void *>() );

    //m_item = 0 if there is no item a that point

    if(m_item){
        context_menu = new DeviceActionsMenu(m_item, this, this);
        context_menu->exec(QCursor::pos());
    }
    else{
        context_menu = new DeviceActionsMenu(NULL, this, this);
        context_menu->exec(QCursor::pos());
    }
}

void DeviceChartSeg::mkfsPartition()
{
    if( make_fs(m_partition) )
        MainWindow->reRun();
}

void DeviceChartSeg::pvcreatePartition()
{
    if( create_pv(m_pv_name) )
	MainWindow->reRun();
}

void DeviceChartSeg::pvremovePartition()
{
    if( remove_pv(m_pv_name) )
	MainWindow->reRun();
}

void DeviceChartSeg::vgcreatePartition()
{
    if( create_vg(m_pv_name) )
	MainWindow->reRun();
}

void DeviceChartSeg::vgreducePartition()
{
    if( reduce_vg_one( m_item->data(4).toString(), m_pv_name) )
	MainWindow->reRun();
}

void DeviceChartSeg::vgextendPartition(QAction *action)
{
    QString group = action->text();
    group.remove(QChar('&'));

    if( extend_vg(group, m_pv_name) )
	MainWindow->reRun();
}

void DeviceChartSeg::mountPartition()
{
    if( mount_filesystem(m_partition) )
        MainWindow->reRun();
}

void DeviceChartSeg::unmountPartition()
{
    if( unmount_filesystem(m_partition) )
        MainWindow->reRun();
}

void DeviceChartSeg::removePartition()
{
  if( remove_partition(m_partition) )
        MainWindow->reRun();
}

void DeviceChartSeg::addPartition()
{
  if( add_partition(m_partition) )
        MainWindow->reRun();
}
