/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010 Benjamin Scott   <benscott@nwlink.com>
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
#include "mount.h"
#include "unmount.h"
#include "partremove.h"
#include "partadd.h"
#include "physvol.h"
#include "storagedevice.h"
#include "tablecreate.h"
#include "removefs.h"
#include "vgreduce.h"
#include "vgreduceone.h"
#include "vgcreate.h"
#include "vgextend.h"


extern MasterList *master_list;


DeviceActionsMenu::DeviceActionsMenu( StorageDeviceItem *item,
				      DeviceTreeView *view,
				      QWidget *parent) : KMenu(parent)
{

    setup(item);

    connect(m_mkfs_action,       SIGNAL(triggered()), view, SLOT(mkfsPartition()));
    connect(m_partremove_action, SIGNAL(triggered()), view, SLOT(removePartition()));
    connect(m_partadd_action,    SIGNAL(triggered()), view, SLOT(addPartition()));
    connect(m_partmoveresize_action, SIGNAL(triggered()), view, SLOT(moveresizePartition()));
    connect(m_removefs_action,   SIGNAL(triggered()), view, SLOT(removefsPartition()));
    connect(m_tablecreate_action,SIGNAL(triggered()), view, SLOT(tablecreatePartition()));
    connect(m_vgcreate_action,   SIGNAL(triggered()), view, SLOT(vgcreatePartition()));
    connect(m_vgreduce_action,   SIGNAL(triggered()), view, SLOT(vgreducePartition()));
    connect(m_mount_action,      SIGNAL(triggered()), view, SLOT(mountPartition()));
    connect(m_unmount_action,    SIGNAL(triggered()), view, SLOT(unmountPartition()));

    connect(m_vgextend_menu, SIGNAL(triggered(QAction*)), 
	    view, SLOT(vgextendPartition(QAction*)));

}

DeviceActionsMenu::DeviceActionsMenu( StorageDeviceItem *item,
				      DeviceChartSeg *segment,
				      QWidget *parent) : KMenu(parent)
{

    setup(item);

    connect(m_mkfs_action,       SIGNAL(triggered()), segment, SLOT(mkfsPartition()));
    connect(m_partremove_action, SIGNAL(triggered()), segment, SLOT(removePartition()));
    connect(m_partadd_action,    SIGNAL(triggered()), segment, SLOT(addPartition()));
    connect(m_partmoveresize_action, SIGNAL(triggered()), segment, SLOT(moveresizePartition()));
    connect(m_removefs_action,   SIGNAL(triggered()), segment, SLOT(removefsPartition()));
    connect(m_vgcreate_action,   SIGNAL(triggered()), segment, SLOT(vgcreatePartition()));
    //    connect(m_tablecreate_action,SIGNAL(triggered()), segment, SLOT(tablecreatePartition()));
    connect(m_vgreduce_action,   SIGNAL(triggered()), segment, SLOT(vgreducePartition()));
    connect(m_mount_action,      SIGNAL(triggered()), segment, SLOT(mountPartition()));
    connect(m_unmount_action,    SIGNAL(triggered()), segment, SLOT(unmountPartition()));

    connect(m_vgextend_menu, SIGNAL(triggered(QAction*)), 
	    segment, SLOT(vgextendPartition(QAction*)));

}

void DeviceActionsMenu::setup(StorageDeviceItem *item)
{
    StoragePartition* partition = NULL;
    StorageDevice* device = NULL;
    QStringList group_names;
    KMenu *filesystem_ops = new KMenu( i18n("Filesystem operations"), this);
    m_vgextend_menu       = new KMenu( i18n("Extend volume group"), this);
    m_mkfs_action       = new KAction( i18n("Make filesystem"), this);
    m_partadd_action    = new KAction( i18n("Add disk partition"), this);
    m_partmoveresize_action = new KAction( i18n("Move or resize disk partition"), this);
    m_partremove_action = new KAction( i18n("Remove disk partition"), this);
    m_removefs_action   = new KAction( i18n("Remove filesystem"), this);
    m_vgcreate_action   = new KAction( i18n("Create volume group"), this);
    m_tablecreate_action= new KAction( i18n("Create new partition table"), this);
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
    addMenu(filesystem_ops);
    filesystem_ops->addAction(m_mount_action);
    filesystem_ops->addAction(m_unmount_action);
    filesystem_ops->addSeparator();
    filesystem_ops->addAction(m_mkfs_action);
    filesystem_ops->addAction(m_removefs_action);

    group_names = master_list->getVolumeGroupNames();
    for(int x = 0; x < group_names.size(); x++){
        vgextend_actions.append(new QAction(group_names[x], this));
        m_vgextend_menu->addAction(vgextend_actions[x]);
    }

    if(item){

        setEnabled(true);
        device = (StorageDevice *) (( item->dataAlternate(1)).value<void *>() );

        if( ( item->dataAlternate(0)).canConvert<void *>() ){  // its a partition
            partition = (StoragePartition *) (( item->dataAlternate(0)).value<void *>() );
            m_tablecreate_action->setEnabled(false);
	    m_mount_action->setEnabled( partition->isMountable() );
            m_unmount_action->setEnabled( partition->isMounted() );

            if( partition->getPedType() & 0x04 ){    // freespace
                m_mkfs_action->setEnabled(false);
                m_partremove_action->setEnabled(false);
                m_partmoveresize_action->setEnabled(false);
                m_partadd_action->setEnabled(true);
                m_removefs_action->setEnabled(false);
                m_vgcreate_action->setEnabled(false);
                m_vgextend_menu->setEnabled(false);
                m_vgreduce_action->setEnabled(false);
            }
            else if( partition->getPedType() & 0x02 ){  // extended partition
                m_mkfs_action->setEnabled(false);
                if( partition->isEmpty() )
                    m_partadd_action->setEnabled(true);
                else
                    m_partadd_action->setEnabled(false);
                m_partmoveresize_action->setEnabled(false);
                m_removefs_action->setEnabled(false);
                m_vgcreate_action->setEnabled(false);
                m_vgextend_menu->setEnabled(false);
                m_vgreduce_action->setEnabled(false);
                if( partition->isEmpty() )
                    m_partremove_action->setEnabled(true);
                else
                    m_partremove_action->setEnabled(false);
            }
            else if( partition->isPV() ){
                m_mkfs_action->setEnabled(false);
                m_partremove_action->setEnabled(false);
                if( partition->isBusy() )        
                    m_partmoveresize_action->setEnabled(false);
                else
                    m_partmoveresize_action->setEnabled(true);
                m_partadd_action->setEnabled(false);
                m_removefs_action->setEnabled(false);
                m_vgcreate_action->setEnabled(false);
                m_vgextend_menu->setEnabled(false);
                if( partition->getPhysicalVolume()->getPercentUsed() == 0 )
                    m_vgreduce_action->setEnabled(true);
                else
                    m_vgreduce_action->setEnabled(false);
            }
            else if( partition->isNormal() || partition->isLogical() ){
                if( partition->isMounted() || partition->isBusy() ){ 
                    m_partremove_action->setEnabled(false);
                    m_partmoveresize_action->setEnabled(false);
                    m_mkfs_action->setEnabled(false);
                    m_removefs_action->setEnabled(false);
                    m_vgcreate_action->setEnabled(false);
                    m_vgextend_menu->setEnabled(false);
                }
                else{                                               // not mounted or busy
                    m_partremove_action->setEnabled(true);
                    m_partmoveresize_action->setEnabled(true);
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
            m_mount_action->setEnabled(false);
            m_unmount_action->setEnabled(false);

            if( device->isPhysicalVolume() ){
                m_tablecreate_action->setEnabled(false);
                m_mkfs_action->setEnabled(false);
                m_partremove_action->setEnabled(false);
                m_partmoveresize_action->setEnabled(false);
                m_partadd_action->setEnabled(false);
                m_removefs_action->setEnabled(false);
                m_vgcreate_action->setEnabled(false);
                m_vgextend_menu->setEnabled(false);
                if( device->getPhysicalVolume()->getPercentUsed() == 0 )   
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

                if( device->isBusy() || device->getRealPartitionCount() != 0 ){
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
