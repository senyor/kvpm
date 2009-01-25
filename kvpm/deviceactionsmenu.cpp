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
#include "deviceactionsmenu.h"
#include "devicesizechartseg.h"
#include "mkfs.h"
#include "mount.h"
#include "unmount.h"
#include "pvcreate.h"
#include "pvremove.h"
#include "partremove.h"
#include "partadd.h"
#include "tablecreate.h"
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
    connect(m_pvcreate_action,   SIGNAL(triggered()), view, SLOT(pvcreatePartition()));
    connect(m_pvremove_action,   SIGNAL(triggered()), view, SLOT(pvremovePartition()));
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
    connect(m_pvcreate_action,   SIGNAL(triggered()), segment, SLOT(pvcreatePartition()));
    connect(m_pvremove_action,   SIGNAL(triggered()), segment, SLOT(pvremovePartition()));
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
    QStringList group_names;
     
    m_vgextend_menu     = new KMenu( i18n("Extend volume group"), this);
    m_mkfs_action       = new KAction( i18n("Make filesystem"), this);
    m_partadd_action    = new KAction( i18n("Add disk partition"), this);
    m_partremove_action = new KAction( i18n("Remove disk partition"), this);
    m_pvcreate_action   = new KAction( i18n("Create physical volume"), this);
    m_pvremove_action   = new KAction( i18n("Remove physical volume"), this);
    m_vgcreate_action   = new KAction( i18n("Create volume group"), this);
    m_tablecreate_action= new KAction( i18n("Create new partition table"), this);
    m_vgreduce_action   = new KAction( i18n("Remove from volume group"), this);
    m_mount_action      = new KAction( i18n("Mount filesystem"), this);
    m_unmount_action    = new KAction( i18n("Unmount filesystem"), this);
    addAction(m_mkfs_action);
    addAction(m_partremove_action);
    addAction(m_partadd_action);
    addAction(m_pvcreate_action);
    addAction(m_pvremove_action);
    addAction(m_vgcreate_action);
    addAction(m_tablecreate_action);
    addAction(m_vgreduce_action);
    addMenu(m_vgextend_menu);
    addAction(m_mount_action);
    addAction(m_unmount_action);

    group_names = master_list->getVolumeGroupNames();
    for(int x = 0; x < group_names.size(); x++){
        vgextend_actions.append(new QAction(group_names[x], this));
        m_vgextend_menu->addAction(vgextend_actions[x]);
    }

    
    if(item){

        setEnabled(true);

	m_tablecreate_action->setEnabled(false);

	if(item->dataAlternate(7).toString() == "Yes"){        // yes = is mountable
	    m_mount_action->setEnabled(true);

	    if(item->data(7) != "")                            // "" = not mounted
	        m_unmount_action->setEnabled(true);
	    else
	        m_unmount_action->setEnabled(false);
	}
	else{
	    m_mount_action->setEnabled(false);
	    m_unmount_action->setEnabled(false);
	}

	if(item->data(1) == "freespace" || 
	   item->data(1) == "freespace (logical)")
	  {
	    m_pvcreate_action->setEnabled(false);
	    m_mkfs_action->setEnabled(false);
	    m_partremove_action->setEnabled(false);
	    m_partadd_action->setEnabled(true);
	    m_pvremove_action->setEnabled(false);
	    m_vgcreate_action->setEnabled(false);
	    m_vgextend_menu->setEnabled(false);
	    m_vgreduce_action->setEnabled(false);
	}
	else if(item->data(1) == "extended" && item->dataAlternate(1) == "empty"){
	    m_pvcreate_action->setEnabled(false);
	    m_mkfs_action->setEnabled(false);
	    m_partadd_action->setEnabled(false);
	    m_partremove_action->setEnabled(true);
	    m_pvremove_action->setEnabled(false);
	    m_vgcreate_action->setEnabled(false);
	    m_vgextend_menu->setEnabled(false);
	    m_vgreduce_action->setEnabled(false);
	}
	else if(item->data(1) == "extended" && item->dataAlternate(1) != "empty"){
	    m_pvcreate_action->setEnabled(false);
	    m_mkfs_action->setEnabled(false);
	    m_partadd_action->setEnabled(false);
	    m_partremove_action->setEnabled(false);
	    m_pvremove_action->setEnabled(false);
	    m_vgcreate_action->setEnabled(false);
	    m_vgextend_menu->setEnabled(false);
	    m_vgreduce_action->setEnabled(false);
	}
	else if(item->data(6) == "yes"){
	    m_pvcreate_action->setEnabled(false);
	    m_partremove_action->setEnabled(false);
	    m_partadd_action->setEnabled(false);
	    m_mkfs_action->setEnabled(false);
	    m_pvremove_action->setEnabled(false);
	    m_vgcreate_action->setEnabled(false);
	    m_vgextend_menu->setEnabled(false);
	    m_vgreduce_action->setEnabled(false);
	}
	else if( (item->data(4) == "physical volume") && (item->data(5) == "" ) ){
	    m_pvcreate_action->setEnabled(false);
	    m_mkfs_action->setEnabled(false);
	    m_partremove_action->setEnabled(false);
	    m_partadd_action->setEnabled(false);
	    m_pvremove_action->setEnabled(true);
	    m_vgcreate_action->setEnabled(true);
	    m_vgextend_menu->setEnabled(true);
	    m_vgreduce_action->setEnabled(false);
	}
	else if( (item->data(4) == "physical volume") && (item->data(5) != "" ) ){
	    m_pvcreate_action->setEnabled(false);
	    m_mkfs_action->setEnabled(false);
	    m_partremove_action->setEnabled(false);
	    m_partadd_action->setEnabled(false);
	    m_pvremove_action->setEnabled(false);
	    m_vgcreate_action->setEnabled(false);
	    m_vgextend_menu->setEnabled(false);
	    if( item->dataAlternate(3) == 0 )
		m_vgreduce_action->setEnabled(true);
	    else
		m_vgreduce_action->setEnabled(false);
	}
	else if(item->data(1) == "logical" || item->data(1) == "normal"){

	    if(item->data(6) == "yes")
   	        m_partremove_action->setEnabled(false);
	    else
	        m_partremove_action->setEnabled(true);

	    m_partadd_action->setEnabled(false);
	    m_pvcreate_action->setEnabled(true);
	    m_pvremove_action->setEnabled(false);
	    m_mkfs_action->setEnabled(true);
	    m_vgcreate_action->setEnabled(false);
	    m_vgextend_menu->setEnabled(false);
	    m_vgreduce_action->setEnabled(false);
	}
	else{
	    m_partremove_action->setEnabled(false);
	    m_partadd_action->setEnabled(false);
	    m_pvcreate_action->setEnabled(false);
	    m_pvremove_action->setEnabled(false);
	    m_mkfs_action->setEnabled(false);
	    m_vgcreate_action->setEnabled(false);
	    m_vgextend_menu->setEnabled(false);
	    m_vgreduce_action->setEnabled(false);

	    if( item->dataAlternate(1) == "busy" )
	        m_tablecreate_action->setEnabled(false);
	    else
	        m_tablecreate_action->setEnabled(true);
	}
    }
    else
	setEnabled(false);  // if item points to NULL, do nothing
}
