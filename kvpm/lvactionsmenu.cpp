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

#include "lvactionsmenu.h"
#include "logvol.h"
#include "vgtree.h"
#include "lvsizechartseg.h"

LVActionsMenu::LVActionsMenu(LogVol *logicalVolume, 
			     VGTree *volumeGroupTree, 
			     QWidget *parent) : KMenu(parent)
{

    setup(logicalVolume);
    
    connect(lv_mkfs_action, SIGNAL(triggered()), 
	    volumeGroupTree, SLOT(mkfsLogicalVolume()));

    connect(lv_reduce_action, SIGNAL(triggered()), 
	    volumeGroupTree, SLOT(reduceLogicalVolume()));

    connect(lv_remove_action, SIGNAL(triggered()), 
	    volumeGroupTree, SLOT(removeLogicalVolume()));

    connect(lv_rename_action, SIGNAL(triggered()), 
	    volumeGroupTree, SLOT(renameLogicalVolume()));

    connect(lv_create_action, SIGNAL(triggered()), 
	    volumeGroupTree, SLOT(createLogicalVolume()));

    connect(snap_create_action, SIGNAL(triggered()), 
	    volumeGroupTree, SLOT(createSnapshot()));

    connect(lv_extend_action, SIGNAL(triggered()), 
	    volumeGroupTree, SLOT(extendLogicalVolume()));

    connect(pv_move_action, SIGNAL(triggered()), 
	    volumeGroupTree, SLOT(movePhysicalExtents()));

    connect(lv_change_action, SIGNAL(triggered()), 
	    volumeGroupTree, SLOT(changeLogicalVolume()));

    connect(add_mirror_action, SIGNAL(triggered()), 
	    volumeGroupTree, SLOT(addMirror()));

    connect(mount_filesystem_action, SIGNAL(triggered()), 
	    volumeGroupTree, SLOT(mountFilesystem()));

    connect(unmount_filesystem_action, SIGNAL(triggered()), 
	    volumeGroupTree, SLOT(unmountFilesystem()));

    connect(remove_mirror_action, SIGNAL(triggered()), 
	    volumeGroupTree, SLOT(removeMirror()));

    connect(remove_mirror_leg_action, SIGNAL(triggered()), 
	    volumeGroupTree, SLOT(removeMirrorLeg()));

}

LVActionsMenu::LVActionsMenu(LogVol *logicalVolume, 
			     LVChartSeg *ChartSeg,
			     QWidget *parent) : KMenu(parent)
{
    setup(logicalVolume);
    
    connect(lv_mkfs_action, SIGNAL(triggered()), 
	    ChartSeg, SLOT(mkfsLogicalVolume()));

    connect(lv_reduce_action, SIGNAL(triggered()), 
	    ChartSeg, SLOT(reduceLogicalVolume()));

    connect(lv_remove_action, SIGNAL(triggered()), 
	    ChartSeg, SLOT(removeLogicalVolume()));

    connect(lv_rename_action, SIGNAL(triggered()), 
	    ChartSeg, SLOT(renameLogicalVolume()));

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
    lv_mkfs_action   = new KAction( i18n("Make filesystem..."), this);
    lv_reduce_action = new KAction( i18n("Reduce logical volume..."), this);
    lv_remove_action = new KAction( i18n("Remove logical volume..."), this);
    lv_rename_action = new KAction( i18n("Rename logical volume..."), this);
    lv_create_action = new KAction( i18n("Create logical volume..."), this);
    snap_create_action = new KAction( i18n("Create snapshot..."), this);
    lv_extend_action   = new KAction( i18n("Extend logical volume..."), this);
    pv_move_action     = new KAction( i18n("Move physical extents..."), this);
    lv_change_action   = new KAction( i18n("Change attributes or tags..."), this);
    add_mirror_action  = new KAction( i18n("Add leg or change mirror..."), this);
    mount_filesystem_action   = new KAction( i18n("Mount filesystem..."), this);
    unmount_filesystem_action = new KAction( i18n("Unmount filesystem..."), this);
    remove_mirror_action      = new KAction( i18n("Remove mirror leg..."), this);
    remove_mirror_leg_action  = new KAction( i18n("Remove this mirror leg..."), this);

    addAction(lv_create_action);
    addAction(lv_remove_action);
    addAction(lv_rename_action);
    addAction(snap_create_action);
    addAction(lv_reduce_action);
    addAction(lv_extend_action);
    addAction(lv_change_action);
    addAction(pv_move_action);

    KMenu *mirror_menu = new KMenu( i18n("Mirror operations"), this);
    addMenu(mirror_menu);
    mirror_menu->addAction(add_mirror_action);
    mirror_menu->addAction(remove_mirror_action);
    mirror_menu->addAction(remove_mirror_leg_action);

    filesystem_menu = new KMenu( i18n("Filesystem operations"), this);
    addMenu(filesystem_menu);
    filesystem_menu->addAction(mount_filesystem_action);
    filesystem_menu->addAction(unmount_filesystem_action);
    filesystem_menu->addSeparator();
    filesystem_menu->addAction(lv_mkfs_action);

    if( lv ){
	if(  lv->isWritable()  && !lv->isLocked() && !lv->isVirtual() && 
	    !lv->isMirrorLeg() && !lv->isMirrorLog() ){

	    if( lv->isMounted() ){
		lv_mkfs_action->setEnabled(false);
		lv_reduce_action->setEnabled(false);
                lv_extend_action->setEnabled(true);
		lv_remove_action->setEnabled(false);
		unmount_filesystem_action->setEnabled(true);
		mount_filesystem_action->setEnabled(true);
	    }
	    else{
		lv_mkfs_action->setEnabled(true);
		lv_reduce_action->setEnabled(true);
                lv_extend_action->setEnabled(true);
		lv_remove_action->setEnabled(true);
		unmount_filesystem_action->setEnabled(false);
		mount_filesystem_action->setEnabled(true);
	    }

            if( lv->isSnap() || lv->isOrigin() ){
                add_mirror_action->setEnabled(false);
		remove_mirror_action->setEnabled(false);

		if( lv->isOrigin() && lv->isActive() )
		  lv_extend_action->setEnabled(false);
		else
		  lv_extend_action->setEnabled(true);

		lv_reduce_action->setEnabled(false);
                pv_move_action->setEnabled(false);

                if( lv->isSnap() ){
                    snap_create_action->setEnabled(false);
                    lv_extend_action->setEnabled(true);
                    lv_reduce_action->setEnabled(true);
                }
                else
                    snap_create_action->setEnabled(true);

                mirror_menu->setEnabled(false);
            }
            else if( lv->isMirror() ){
                add_mirror_action->setEnabled(true);
		remove_mirror_action->setEnabled(true);
                lv_extend_action->setEnabled(true);
                lv_reduce_action->setEnabled(true);
                pv_move_action->setEnabled(false);
            }
	    else{
                add_mirror_action->setEnabled(true);
		remove_mirror_action->setEnabled(false);
                pv_move_action->setEnabled(true);
                snap_create_action->setEnabled(true);
            }

	    lv_change_action->setEnabled(true);
	    lv_rename_action->setEnabled(true);
	    remove_mirror_leg_action->setEnabled(false);
	    filesystem_menu->setEnabled(true);
	}
        else if( lv->isOrphan() ){
	    lv_mkfs_action->setEnabled(false);
	    lv_remove_action->setEnabled(true);
	    unmount_filesystem_action->setEnabled(false);
	    mount_filesystem_action->setEnabled(false);
	    add_mirror_action->setEnabled(false);
	    lv_change_action->setEnabled(false);
	    lv_extend_action->setEnabled(false);
	    lv_reduce_action->setEnabled(false);
	    lv_rename_action->setEnabled(false);
	    pv_move_action->setEnabled(false);
	    remove_mirror_action->setEnabled(false);
	    remove_mirror_leg_action->setEnabled(false);
	    snap_create_action->setEnabled(false);
	    filesystem_menu->setEnabled(false);
        }
	else if( lv->isPvmove() ){
	    lv_mkfs_action->setEnabled(false);
	    lv_remove_action->setEnabled(false);
	    unmount_filesystem_action->setEnabled(false);
	    mount_filesystem_action->setEnabled(false);
	    add_mirror_action->setEnabled(false);
	    lv_change_action->setEnabled(false);
	    lv_extend_action->setEnabled(false);
	    lv_reduce_action->setEnabled(false);
	    lv_rename_action->setEnabled(false);
	    pv_move_action->setEnabled(false);
	    remove_mirror_action->setEnabled(false);
	    remove_mirror_leg_action->setEnabled(false);
	    snap_create_action->setEnabled(false);
	    filesystem_menu->setEnabled(false);
	}
	else if( lv->isMirrorLeg() || lv->isMirrorLog() ){
	    lv_mkfs_action->setEnabled(false);
	    lv_remove_action->setEnabled(false);
	    unmount_filesystem_action->setEnabled(false);
	    mount_filesystem_action->setEnabled(false);
	    add_mirror_action->setEnabled(false);
	    lv_change_action->setEnabled(false);
	    lv_extend_action->setEnabled(false);
	    lv_reduce_action->setEnabled(false);
	    pv_move_action->setEnabled(false);
	    remove_mirror_action->setEnabled(false);
	    lv_rename_action->setEnabled(false);

	    if( !lv->isMirrorLog() )
		remove_mirror_leg_action->setEnabled(true);
	    else{
		remove_mirror_leg_action->setEnabled(false);
                mirror_menu->setEnabled(false);
            }
	    snap_create_action->setEnabled(false);
	    filesystem_menu->setEnabled(false);
	}
	else if( !(lv->isWritable()) && lv->isLocked() ){

	    if( lv->isMounted() )
		unmount_filesystem_action->setEnabled(true);
	    else
		unmount_filesystem_action->setEnabled(false);

            mount_filesystem_action->setEnabled(true);
	    lv_mkfs_action->setEnabled(false);
	    lv_remove_action->setEnabled(false);
	    add_mirror_action->setEnabled(false);
	    lv_change_action->setEnabled(true);
	    lv_extend_action->setEnabled(false);
	    lv_reduce_action->setEnabled(false);
	    pv_move_action->setEnabled(false);
	    remove_mirror_action->setEnabled(false);
	    remove_mirror_leg_action->setEnabled(false);
	    snap_create_action->setEnabled(false);
	    filesystem_menu->setEnabled(true);
	}
	else if( lv->isWritable() && lv->isLocked() ){

	    if( lv->isMounted() )
		unmount_filesystem_action->setEnabled(true);
	    else
		unmount_filesystem_action->setEnabled(false);

            mount_filesystem_action->setEnabled(true);
	    lv_mkfs_action->setEnabled(true);
	    lv_remove_action->setEnabled(false);
	    add_mirror_action->setEnabled(false);
	    lv_change_action->setEnabled(true);
	    lv_extend_action->setEnabled(false);
	    lv_reduce_action->setEnabled(false);
	    pv_move_action->setEnabled(false);
	    remove_mirror_action->setEnabled(false);
	    remove_mirror_leg_action->setEnabled(false);
	    snap_create_action->setEnabled(false);
	    filesystem_menu->setEnabled(true);
	}
	else{
	    if( lv->isMounted() ){
		lv_remove_action->setEnabled(false);
		unmount_filesystem_action->setEnabled(true);
	    }
	    else{
		lv_remove_action->setEnabled(true);
		unmount_filesystem_action->setEnabled(false);
	    }

            if( lv->isSnap() || lv->isOrigin() ){
                add_mirror_action->setEnabled(false);
		remove_mirror_action->setEnabled(false);
                pv_move_action->setEnabled(false);

                if( lv->isSnap() )
                    snap_create_action->setEnabled(false);
                else
                    snap_create_action->setEnabled(true);

                mirror_menu->setEnabled(false);
            }
            else if( lv->isMirror() ){
                add_mirror_action->setEnabled(true);
		remove_mirror_action->setEnabled(true);
                pv_move_action->setEnabled(false);
                snap_create_action->setEnabled(false);
            }
	    else{
                add_mirror_action->setEnabled(true);
		remove_mirror_action->setEnabled(false);
                pv_move_action->setEnabled(true);
                snap_create_action->setEnabled(true);
            }

            mount_filesystem_action->setEnabled(true);
	    lv_mkfs_action->setEnabled(false);
	    lv_reduce_action->setEnabled(false);
	    lv_change_action->setEnabled(true);
	    lv_extend_action->setEnabled(false);
	    remove_mirror_leg_action->setEnabled(false);
	    filesystem_menu->setEnabled(true);
	}

        if( !lv->isActive() ){
            lv_mkfs_action->setEnabled(false);
            unmount_filesystem_action->setEnabled(false);
            mount_filesystem_action->setEnabled(false);
            filesystem_menu->setEnabled(false);
            snap_create_action->setEnabled(false);
        }
    }
    else{
	lv_mkfs_action->setEnabled(false);
	lv_remove_action->setEnabled(false);
	unmount_filesystem_action->setEnabled(false);
	mount_filesystem_action->setEnabled(false);
	add_mirror_action->setEnabled(false);
	lv_change_action->setEnabled(false);
	lv_extend_action->setEnabled(false);
	lv_reduce_action->setEnabled(false);
	lv_rename_action->setEnabled(false);
	pv_move_action->setEnabled(false);
	remove_mirror_action->setEnabled(false);
	remove_mirror_leg_action->setEnabled(false);
	snap_create_action->setEnabled(false);
	filesystem_menu->setEnabled(false);
    }
}
