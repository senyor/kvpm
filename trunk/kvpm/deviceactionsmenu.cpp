/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
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
#include "deviceactionsmenu.h"
#include "devicesizechartseg.h"
#include "mkfs.h"
#include "maxfs.h"
#include "mount.h"
#include "unmount.h"
#include "masterlist.h"
#include "partremove.h"
#include "partadd.h"
#include "partmoveresize.h"
#include "physvol.h"
#include "storagedevice.h"
#include "storagepartition.h"
#include "tablecreate.h"
#include "topwindow.h"
#include "removefs.h"
#include "vgreduce.h"
#include "vgreduceone.h"
#include "vgcreate.h"
#include "vgextend.h"

extern MasterList *master_list;


DeviceActionsMenu::DeviceActionsMenu( StorageDeviceItem *item, QWidget *parent) : KMenu(parent)
{
    setup(item);

    connect(m_maxfs_action,      SIGNAL(triggered()), this, SLOT(maxfsPartition()));
    connect(m_mkfs_action,       SIGNAL(triggered()), this, SLOT(mkfsPartition()));
    connect(m_partremove_action, SIGNAL(triggered()), this, SLOT(removePartition()));
    connect(m_partadd_action,    SIGNAL(triggered()), this, SLOT(addPartition()));
    connect(m_partmoveresize_action, SIGNAL(triggered()), this, SLOT(moveresizePartition()));
    connect(m_removefs_action,   SIGNAL(triggered()), this, SLOT(removefsPartition()));
    connect(m_vgcreate_action,   SIGNAL(triggered()), this, SLOT(vgcreatePartition()));
    connect(m_tablecreate_action,SIGNAL(triggered()), this, SLOT(tablecreatePartition()));
    connect(m_vgreduce_action,   SIGNAL(triggered()), this, SLOT(vgreducePartition()));
    connect(m_mount_action,      SIGNAL(triggered()), this, SLOT(mountPartition()));
    connect(m_unmount_action,    SIGNAL(triggered()), this, SLOT(unmountPartition()));

    connect(m_vgextend_menu, SIGNAL(triggered(QAction*)), this, SLOT(vgextendPartition(QAction*)));
}

void DeviceActionsMenu::setup(StorageDeviceItem *item)
{
    QStringList group_names;
    KMenu *filesystem_menu = new KMenu( i18n("Filesystem operations"), this);
    m_vgextend_menu       = new KMenu( i18n("Extend volume group"), this);
    m_maxfs_action      = new KAction( i18n("Extend filesystem to fill partition"), this);
    m_mkfs_action       = new KAction( i18n("Make filesystem"), this);
    m_partadd_action    = new KAction( i18n("Add disk partition"), this);
    m_partmoveresize_action = new KAction( i18n("Move or resize disk partition"), this);
    m_partremove_action = new KAction( i18n("Remove disk partition"), this);
    m_removefs_action   = new KAction( i18n("Remove filesystem"), this);
    m_vgcreate_action   = new KAction( i18n("Create volume group"), this);
    m_tablecreate_action= new KAction( i18n("Create or destroy partition table"), this);
    m_vgreduce_action   = new KAction( i18n("Remove from volume group"), this);
    m_mount_action      = new KAction( i18n("Mount filesystem"), this);
    m_unmount_action    = new KAction( i18n("Unmount filesystem"), this);
    addAction(m_tablecreate_action);
    addSeparator();
    addAction(m_partremove_action);
    addAction(m_partadd_action);
    addAction(m_partmoveresize_action);
    addSeparator();
    addAction(m_vgcreate_action);
    addAction(m_vgreduce_action);
    addMenu(m_vgextend_menu);
    addSeparator();
    addMenu(filesystem_menu);
    filesystem_menu->addAction(m_mount_action);
    filesystem_menu->addAction(m_unmount_action);
    filesystem_menu->addSeparator();
    filesystem_menu->addAction(m_maxfs_action);
    filesystem_menu->addAction(m_mkfs_action);
    filesystem_menu->addAction(m_removefs_action);

    m_vg_name  = item->data(5).toString(); // only set if this is a pv in a vg

    group_names = master_list->getVolumeGroupNames();
    for(int x = 0; x < group_names.size(); x++){
        vgextend_actions.append(new QAction(group_names[x], this));
        m_vgextend_menu->addAction(vgextend_actions[x]);
    }

    if(item){

        setEnabled(true);
        m_dev = (StorageDevice *) (( item->dataAlternate(1)).value<void *>() );
        filesystem_menu->setEnabled(false);

        if( ( item->dataAlternate(0)).canConvert<void *>() ){  // its a partition
            m_part = (StoragePartition *) (( item->dataAlternate(0)).value<void *>() );
            m_tablecreate_action->setEnabled(false);
	    m_mount_action->setEnabled( m_part->isMountable() );
            m_unmount_action->setEnabled( m_part->isMounted() );

            if( m_part->getPedType() & 0x04 ){    // freespace
                m_maxfs_action->setEnabled(false);
                m_mkfs_action->setEnabled(false);
                m_partremove_action->setEnabled(false);
                m_partmoveresize_action->setEnabled(false);
                m_partadd_action->setEnabled(true);
                m_removefs_action->setEnabled(false);
                m_vgcreate_action->setEnabled(false);
                m_vgextend_menu->setEnabled(false);
                m_vgreduce_action->setEnabled(false);
            }
            else if( m_part->getPedType() & 0x02 ){  // extended partition
                m_maxfs_action->setEnabled(false);
                m_mkfs_action->setEnabled(false);
                if( m_part->isEmpty() )
                    m_partadd_action->setEnabled(true);
                else
                    m_partadd_action->setEnabled(false);
                m_partmoveresize_action->setEnabled(false);
                m_removefs_action->setEnabled(false);
                m_vgcreate_action->setEnabled(false);
                m_vgextend_menu->setEnabled(false);
                m_vgreduce_action->setEnabled(false);
                if( m_part->isEmpty() )
                    m_partremove_action->setEnabled(true);
                else
                    m_partremove_action->setEnabled(false);
            }
            else if( m_part->isPV() ){
                m_maxfs_action->setEnabled(false);
                m_mkfs_action->setEnabled(false);
                m_partremove_action->setEnabled(false);
                if( m_part->isBusy() )        
                    m_partmoveresize_action->setEnabled(false);
                else
                    m_partmoveresize_action->setEnabled(true);
                m_partadd_action->setEnabled(false);
                m_removefs_action->setEnabled(false);
                m_vgcreate_action->setEnabled(false);
                m_vgextend_menu->setEnabled(false);
                if( m_part->getPhysicalVolume()->getPercentUsed() == 0 )
                    m_vgreduce_action->setEnabled(true);
                else
                    m_vgreduce_action->setEnabled(false);
            }
            else if( m_part->isNormal() || m_part->isLogical() ){
                filesystem_menu->setEnabled(true);

                if( m_part->isMounted() || m_part->isBusy() ){ 
                    m_partremove_action->setEnabled(false);
                    m_partmoveresize_action->setEnabled(false);
                    m_mkfs_action->setEnabled(false);
                    m_maxfs_action->setEnabled(false);
                    m_removefs_action->setEnabled(false);
                    m_vgcreate_action->setEnabled(false);
                    m_vgextend_menu->setEnabled(false);
                }
                else{                                               // not mounted or busy
                    m_partremove_action->setEnabled(true);
                    m_partmoveresize_action->setEnabled(true);
                    m_maxfs_action->setEnabled(true);
                    m_mkfs_action->setEnabled(true);
                    m_removefs_action->setEnabled(true);
                    m_vgcreate_action->setEnabled(true);
                    if(group_names.size())
                        m_vgextend_menu->setEnabled(true);
                    else
                        m_vgextend_menu->setEnabled(false);
                }
                m_partadd_action->setEnabled(false);
                m_vgreduce_action->setEnabled(false);
            }
        }
        else { // its a whole device
            m_part = NULL;
            m_mount_action->setEnabled(false);
            m_unmount_action->setEnabled(false);
            m_maxfs_action->setEnabled(false);

            if( m_dev->isPhysicalVolume() ){
                m_tablecreate_action->setEnabled(false);
                m_mkfs_action->setEnabled(false);
                m_partremove_action->setEnabled(false);
                m_partmoveresize_action->setEnabled(false);
                m_partadd_action->setEnabled(false);
                m_removefs_action->setEnabled(false);
                m_vgcreate_action->setEnabled(false);
                m_vgextend_menu->setEnabled(false);
                if( m_dev->getPhysicalVolume()->getPercentUsed() == 0 )   
                    m_vgreduce_action->setEnabled(true);
                else
                    m_vgreduce_action->setEnabled(false);
            }
            else{ // not a pv
                m_partremove_action->setEnabled(false);
                m_partmoveresize_action->setEnabled(false);
                m_partadd_action->setEnabled(false);
                m_mkfs_action->setEnabled(false);
                m_removefs_action->setEnabled(false);
                m_vgreduce_action->setEnabled(false);

                if( m_dev->isBusy() || m_dev->getRealPartitionCount() != 0 ){
                    m_tablecreate_action->setEnabled(false);
                    m_vgcreate_action->setEnabled(false);
                    m_vgextend_menu->setEnabled(false);
                }
                else{ 
                    m_tablecreate_action->setEnabled(true);
                    m_vgcreate_action->setEnabled(true);
                    m_vgextend_menu->setEnabled(true);
                }
            }

        }
    }
    else
	setEnabled(false);  // if item points to NULL, do nothing
}

void DeviceActionsMenu::mkfsPartition()
{
    if( make_fs(m_part) )
	    MainWindow->reRun();
}

void DeviceActionsMenu::maxfsPartition()
{
    if( max_fs(m_part) )
	    MainWindow->reRun();
}

void DeviceActionsMenu::removePartition()
{
  if( remove_partition(m_part) )
	MainWindow->reRun();
}

void DeviceActionsMenu::addPartition()
{
  if( add_partition(m_part) )
	MainWindow->reRun();
}

void DeviceActionsMenu::moveresizePartition()
{
  if( moveresize_partition(m_part) )
	MainWindow->reRun();
}

void DeviceActionsMenu::vgcreatePartition()
{
    QString name;
    long long size;

    if(m_part){
        size = m_part->getSize();
        name = m_part->getName();
    }
    else{
        size = m_dev->getSize();
        name = m_dev->getDevicePath();
    }

    if( create_vg(name, size) )
        MainWindow->reRun();
}

void DeviceActionsMenu::tablecreatePartition()
{
    if( create_table( m_dev->getDevicePath() ) )
        MainWindow->reRun();
}

void DeviceActionsMenu::vgreducePartition() // pvs can also be whole devices
{
    QString name;

    if(m_part)
        name = m_part->getName();
    else
        name = m_dev->getDevicePath();

    if( reduce_vg_one(m_vg_name, name) )
	MainWindow->reRun();
}

void DeviceActionsMenu::vgextendPartition(QAction *action)
{
    QString group = action->text();
    group.remove(QChar('&'));
    QString name;
    long long size;

    if(m_part){
        size = m_part->getSize();
        name = m_part->getName();
    }
    else{
        size = m_dev->getSize();
        name = m_dev->getDevicePath();
    }

    if( extend_vg(group, name, size) )
	MainWindow->reRun();
}

void DeviceActionsMenu::mountPartition()
{
    if( mount_filesystem(m_part) )
	MainWindow->reRun();
}

void DeviceActionsMenu::removefsPartition()
{
    if( remove_fs(m_part) )
	MainWindow->reRun();
}

void DeviceActionsMenu::unmountPartition()
{
    if( unmount_filesystem(m_part) )
	MainWindow->reRun();
}

