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

#include "lvactionsmenu.h"
#include "logvol.h"
#include "vgtree.h"
#include "lvsizechartseg.h"

LVActionsMenu::LVActionsMenu(LogVol *LogicalVolume, 
			     VGTree *VolumeGroupTree, 
			     QWidget *parent) : KMenu(parent)
{
    setup(LogicalVolume);
    
    connect(lv_mkfs_action, SIGNAL(triggered()), 
	    VolumeGroupTree, SLOT(mkfsLogicalVolume()));
    connect(lv_reduce_action, SIGNAL(triggered()), 
	    VolumeGroupTree, SLOT(reduceLogicalVolume()));
    connect(lv_remove_action, SIGNAL(triggered()), 
	    VolumeGroupTree, SLOT(removeLogicalVolume()));
    connect(lv_create_action, SIGNAL(triggered()), 
	    VolumeGroupTree, SLOT(createLogicalVolume()));
    connect(snap_create_action, SIGNAL(triggered()), 
	    VolumeGroupTree, SLOT(createSnapshot()));
    connect(lv_extend_action, SIGNAL(triggered()), 
	    VolumeGroupTree, SLOT(extendLogicalVolume()));
    connect(pv_move_action, SIGNAL(triggered()), 
	    VolumeGroupTree, SLOT(movePhysicalExtents()));
    connect(lv_change_action, SIGNAL(triggered()), 
	    VolumeGroupTree, SLOT(changeLogicalVolume()));
    connect(add_mirror_action, SIGNAL(triggered()), 
	    VolumeGroupTree, SLOT(addMirror()));
    connect(mount_filesystem_action, SIGNAL(triggered()), 
	    VolumeGroupTree, SLOT(mountFilesystem()));
    connect(unmount_filesystem_action, SIGNAL(triggered()), 
	    VolumeGroupTree, SLOT(unmountFilesystem()));
    connect(remove_mirror_action, SIGNAL(triggered()), 
	    VolumeGroupTree, SLOT(removeMirror()));
}

LVActionsMenu::LVActionsMenu(LogVol *LogicalVolume, 
			     LVChartSeg *ChartSeg,
			     QWidget *parent) : KMenu(parent)
{
    setup(LogicalVolume);
    
    connect(lv_mkfs_action, SIGNAL(triggered()), 
	    ChartSeg, SLOT(mkfsLogicalVolume()));
    connect(lv_reduce_action, SIGNAL(triggered()), 
	    ChartSeg, SLOT(reduceLogicalVolume()));
    connect(lv_remove_action, SIGNAL(triggered()), 
	    ChartSeg, SLOT(removeLogicalVolume()));
    connect(lv_create_action, SIGNAL(triggered()), 
	    ChartSeg, SLOT(createLogicalVolume()));
    connect(snap_create_action, SIGNAL(triggered()), 
	    ChartSeg, SLOT(createSnapshot()));
    connect(lv_extend_action, SIGNAL(triggered()), 
	    ChartSeg, SLOT(extendLogicalVolume()));
    connect(pv_move_action, SIGNAL(triggered()), 
	    ChartSeg, SLOT(movePhysicalExtents()));
    connect(lv_change_action, SIGNAL(triggered()), 
	    ChartSeg, SLOT(changeLogicalVolume()));
    connect(add_mirror_action, SIGNAL(triggered()), 
	    ChartSeg, SLOT(addMirror()));
    connect(mount_filesystem_action, SIGNAL(triggered()), 
	    ChartSeg, SLOT(mountFilesystem()));
    connect(unmount_filesystem_action, SIGNAL(triggered()), 
	    ChartSeg, SLOT(unmountFilesystem()));
    connect(remove_mirror_action, SIGNAL(triggered()), 
	    ChartSeg, SLOT(removeMirror()));
}

void LVActionsMenu::setup(LogVol *lv)
{
    lv_mkfs_action = new KAction("Make filesystem", this);
    lv_reduce_action = new KAction("Reduce logical volume", this);
    lv_remove_action = new KAction("Remove logical volume", this);
    lv_create_action = new KAction("Create logical volume", this);
    snap_create_action = new KAction("Create snapshot", this);
    lv_extend_action = new KAction("Extend logical volume", this);
    pv_move_action = new KAction("Move physical extents", this);
    lv_change_action = new KAction("Change logical volume attributes", this);
    add_mirror_action = new KAction("Add logical volume mirror", this);
    mount_filesystem_action = new KAction("Mount filesystem", this);
    unmount_filesystem_action = new KAction("Unmount filesystem", this);
    remove_mirror_action = new KAction("Remove logical volume mirror", this);

    addAction(lv_create_action);
    addAction(lv_remove_action);
    addAction(snap_create_action);
    addAction(lv_reduce_action);
    addAction(lv_extend_action);
    addAction(pv_move_action);
    addAction(lv_change_action);
    addAction(add_mirror_action);
    addAction(remove_mirror_action);

    filesystem_menu = new KMenu("Filesystem Operations", this);
    addMenu(filesystem_menu);
    filesystem_menu->addAction(mount_filesystem_action);
    filesystem_menu->addAction(unmount_filesystem_action);
    filesystem_menu->addAction(lv_mkfs_action);

    if( lv ){
	if( lv->isWritable() && !(lv->isLocked()) ){
	    if( lv->isMounted() ){
		lv_mkfs_action->setEnabled(FALSE);
		lv_reduce_action->setEnabled(FALSE);
		lv_remove_action->setEnabled(FALSE);
		unmount_filesystem_action->setEnabled(TRUE);
		mount_filesystem_action->setEnabled(FALSE);
	    }
	    else{
		lv_mkfs_action->setEnabled(TRUE);
		lv_reduce_action->setEnabled(TRUE);
		lv_remove_action->setEnabled(TRUE);
		unmount_filesystem_action->setEnabled(FALSE);
		mount_filesystem_action->setEnabled(TRUE);
	    }
	    add_mirror_action->setEnabled(TRUE);
	    lv_change_action->setEnabled(TRUE);
	    lv_extend_action->setEnabled(TRUE);
	    pv_move_action->setEnabled(TRUE);
	    remove_mirror_action->setEnabled(TRUE);
	    snap_create_action->setEnabled(TRUE);
	    filesystem_menu->setEnabled(TRUE);
	}
	else if( lv->isPvmove() ){
	    lv_mkfs_action->setEnabled(FALSE);
	    lv_remove_action->setEnabled(FALSE);
	    unmount_filesystem_action->setEnabled(FALSE);
	    mount_filesystem_action->setEnabled(FALSE);
	    add_mirror_action->setEnabled(FALSE);
	    lv_change_action->setEnabled(FALSE);
	    lv_extend_action->setEnabled(FALSE);
	    lv_reduce_action->setEnabled(FALSE);
	    pv_move_action->setEnabled(FALSE);
	    remove_mirror_action->setEnabled(FALSE);
	    snap_create_action->setEnabled(FALSE);
	    filesystem_menu->setEnabled(FALSE);
	}
	else if( !(lv->isWritable()) && lv->isLocked() ){
	    if( lv->isMounted() ){
		unmount_filesystem_action->setEnabled(TRUE);
		mount_filesystem_action->setEnabled(FALSE);
	    }
	    else{
		unmount_filesystem_action->setEnabled(FALSE);
		mount_filesystem_action->setEnabled(TRUE);
	    }
	    lv_mkfs_action->setEnabled(FALSE);
	    lv_remove_action->setEnabled(FALSE);
	    add_mirror_action->setEnabled(FALSE);
	    lv_change_action->setEnabled(TRUE);
	    lv_extend_action->setEnabled(FALSE);
	    lv_reduce_action->setEnabled(FALSE);
	    pv_move_action->setEnabled(FALSE);
	    remove_mirror_action->setEnabled(FALSE);
	    snap_create_action->setEnabled(FALSE);
	    filesystem_menu->setEnabled(TRUE);
	}
	else if( lv->isWritable() && lv->isLocked() ){
	    if( lv->isMounted() ){
		unmount_filesystem_action->setEnabled(TRUE);
		mount_filesystem_action->setEnabled(FALSE);
	    }
	    else{
		unmount_filesystem_action->setEnabled(FALSE);
		mount_filesystem_action->setEnabled(TRUE);
	    }
	    lv_mkfs_action->setEnabled(TRUE);
	    lv_remove_action->setEnabled(FALSE);
	    add_mirror_action->setEnabled(FALSE);
	    lv_change_action->setEnabled(TRUE);
	    lv_extend_action->setEnabled(FALSE);
	    lv_reduce_action->setEnabled(FALSE);
	    pv_move_action->setEnabled(FALSE);
	    remove_mirror_action->setEnabled(FALSE);
	    snap_create_action->setEnabled(FALSE);
	    filesystem_menu->setEnabled(TRUE);
	}
	else{
	    if( lv->isMounted() ){
		lv_remove_action->setEnabled(FALSE);
		unmount_filesystem_action->setEnabled(TRUE);
		mount_filesystem_action->setEnabled(FALSE);
	    }
	    else{
		lv_remove_action->setEnabled(TRUE);
		unmount_filesystem_action->setEnabled(FALSE);
		mount_filesystem_action->setEnabled(TRUE);
	    }
	    lv_mkfs_action->setEnabled(FALSE);
	    lv_reduce_action->setEnabled(FALSE);
	    add_mirror_action->setEnabled(TRUE);
	    lv_change_action->setEnabled(TRUE);
	    lv_extend_action->setEnabled(FALSE);
	    pv_move_action->setEnabled(TRUE);
	    remove_mirror_action->setEnabled(TRUE);
	    snap_create_action->setEnabled(TRUE);
	    filesystem_menu->setEnabled(TRUE);
	}
    }
    else{
	lv_mkfs_action->setEnabled(FALSE);
	lv_remove_action->setEnabled(FALSE);
	unmount_filesystem_action->setEnabled(FALSE);
	mount_filesystem_action->setEnabled(FALSE);
	add_mirror_action->setEnabled(FALSE);
	lv_change_action->setEnabled(FALSE);
	lv_extend_action->setEnabled(FALSE);
	lv_reduce_action->setEnabled(FALSE);
	pv_move_action->setEnabled(FALSE);
	remove_mirror_action->setEnabled(FALSE);
	snap_create_action->setEnabled(FALSE);
	filesystem_menu->setEnabled(FALSE);
    }
}
