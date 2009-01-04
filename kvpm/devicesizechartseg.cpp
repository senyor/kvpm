/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
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
#include "masterlist.h"
#include "mkfs.h"
#include "processprogress.h"
#include "pvcreate.h"
#include "pvremove.h"
#include "storagepartition.h"
#include "topwindow.h"
#include "vgcreate.h"
#include "vgextend.h"
#include "vgreduce.h"
#include "vgreduceone.h"

extern MasterList *master_list;

DeviceChartSeg::DeviceChartSeg(StorageDeviceItem *storageDeviceItem, QWidget *parent) : 
    QWidget(parent),
    m_item(storageDeviceItem)
{
    QStringList group_names;
    QString use;
    
    m_partition = NULL;
    if( (m_item->dataAlternate(0)).canConvert<void *>() )
	m_partition = (StoragePartition *) (( m_item->dataAlternate(0)).value<void *>() );

    m_pv_name = (m_item->data(0)).toString();
    use =  (m_item->data(4)).toString();
    QPalette *colorset = new QPalette();

    if(m_pv_name == "freespace"){
	colorset->setColor(QPalette::Window, Qt::green);
    }
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
    setAutoFillBackground(true);

    setContextMenuPolicy(Qt::CustomContextMenu);
    m_context_menu  = new QMenu(this);
    m_vgextend_menu = new QMenu( i18n("Extend Volume Group"), this);

    m_mkfs_action     = new QAction( i18n("Make Filesystem"), this);
    m_pvcreate_action = new QAction( i18n("Create physical volume"), this);
    m_pvremove_action = new QAction( i18n("Remove physical volume"), this);
    m_vgcreate_action = new QAction( i18n("Create volume group"), this);
    m_vgreduce_action = new QAction( i18n("Remove from volume group"), this);
    m_mount_action    = new QAction( i18n("Mount filesystem"), this);
    m_unmount_action  = new QAction( i18n("Unmount filesystem"), this);
    
    group_names = master_list->getVolumeGroupNames();

    for(int x = 0; x < group_names.size(); x++){
	m_vgextend_menu->addAction( new QAction(group_names[x], this) );
    }

    m_context_menu->addAction(m_mkfs_action);
    m_context_menu->addAction(m_pvcreate_action);
    m_context_menu->addAction(m_pvremove_action);
    m_context_menu->addAction(m_vgcreate_action);
    m_context_menu->addAction(m_vgreduce_action);
    m_context_menu->addAction(m_mount_action);
    m_context_menu->addAction(m_unmount_action);
    
    m_context_menu->addMenu(m_vgextend_menu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), 
	    this, SLOT(popupContextMenu(QPoint)) );

    connect(m_mkfs_action,     SIGNAL(triggered()), this, SLOT(mkfsPartition()));
    connect(m_pvcreate_action, SIGNAL(triggered()), this, SLOT(pvcreatePartition()));
    connect(m_pvremove_action, SIGNAL(triggered()), this, SLOT(pvremovePartition()));
    connect(m_vgcreate_action, SIGNAL(triggered()), this, SLOT(vgcreatePartition()));
    connect(m_vgreduce_action, SIGNAL(triggered()), this, SLOT(vgreducePartition()));

    connect(m_vgextend_menu, SIGNAL(triggered(QAction*)), 
	    this, SLOT(vgextendPartition(QAction*)));
}

void DeviceChartSeg::popupContextMenu(QPoint point)
{
    (void)point;

    if(m_item->data(6) == "yes"){
	m_mount_action->setEnabled(true);
	m_unmount_action->setEnabled(true);
    }
    else if(m_item->data(6) == "no"){
	m_mount_action->setEnabled(true);
	m_unmount_action->setEnabled(false);
    }
    else{
	m_mount_action->setEnabled(false);
	m_unmount_action->setEnabled(false);
    }
    
    if(m_item->data(0) == "freespace" || m_item->data(1) == "extended" ){
	m_pvcreate_action->setEnabled(false);
	m_mkfs_action->setEnabled(false);
	m_pvremove_action->setEnabled(false);
	m_vgcreate_action->setEnabled(false);
	m_vgextend_menu->setEnabled(false);
	m_vgreduce_action->setEnabled(false);
    }
    else if(m_item->data(6) == "yes"){
	m_pvcreate_action->setEnabled(false);
	m_mkfs_action->setEnabled(false);
	m_pvremove_action->setEnabled(false);
	m_vgcreate_action->setEnabled(false);
	m_vgextend_menu->setEnabled(false);
	m_vgreduce_action->setEnabled(false);
    }
    else if( (m_item->data(4) == "physical volume") && (m_item->data(5) == "" ) ){
	m_pvcreate_action->setEnabled(false);
	m_mkfs_action->setEnabled(false);
	m_pvremove_action->setEnabled(true);
	m_vgcreate_action->setEnabled(true);
	m_vgextend_menu->setEnabled(true);
	m_vgreduce_action->setEnabled(false);
    }
    else if( (m_item->data(4) == "physical volume") && (m_item->data(5) != "" ) ){
	m_pvcreate_action->setEnabled(false);
	m_mkfs_action->setEnabled(false);
	m_pvremove_action->setEnabled(false);
	m_vgcreate_action->setEnabled(false);
	m_vgextend_menu->setEnabled(false);
	if( m_item->dataAlternate(3) == 0 )
	    m_vgreduce_action->setEnabled(true);
	else
	    m_vgreduce_action->setEnabled(false);
    }
    else if(m_item->data(1) == "logical" || m_item->data(1) == "normal"){
	m_pvcreate_action->setEnabled(true);
	m_pvremove_action->setEnabled(false);
	m_mkfs_action->setEnabled(true);
	m_vgcreate_action->setEnabled(false);
	m_vgextend_menu->setEnabled(false);
	m_vgreduce_action->setEnabled(false);
    }
    else{
	m_pvcreate_action->setEnabled(false);
	m_pvremove_action->setEnabled(false);
	m_mkfs_action->setEnabled(false);
	m_vgcreate_action->setEnabled(false);
	m_vgextend_menu->setEnabled(false);
	m_vgreduce_action->setEnabled(false);
    }
    m_context_menu->exec(QCursor::pos());
    m_context_menu->setEnabled(true);
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
