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
#include <KActionCollection>

#include "lvactionsmenu.h"
#include "logvol.h"
#include "vgtree.h"
#include "lvsizechartseg.h"
#include "topwindow.h"
#include "addmirror.h"
#include "lvactionsmenu.h"
#include "lvsizechartseg.h"
#include "lvcreate.h"
#include "lvchange.h"
#include "lvreduce.h"
#include "lvremove.h"
#include "lvrename.h"
#include "mkfs.h"
#include "mount.h"
#include "pvmove.h"
#include "removemirror.h"
#include "unmount.h"
#include "volgroup.h"


LVActionsMenu::LVActionsMenu(LogVol *logicalVolume, VolGroup *volumeGroup, QWidget *parent) : 
    KMenu(parent), m_vg(volumeGroup), m_lv(logicalVolume)
{

    KActionCollection *lv_actions = new KActionCollection(this);
    lv_actions->addAssociatedWidget(this);
    lv_create_action = lv_actions->addAction( "lvcreate", this, SLOT(createLogicalVolume()));
    lv_create_action->setText( i18n("Create logical volume...") );
    lv_reduce_action = lv_actions->addAction( "lvreduce", this, SLOT(reduceLogicalVolume()));
    lv_reduce_action->setText( i18n("Reduce logical volume...") );
    lv_remove_action = lv_actions->addAction("lvremove", this, SLOT(removeLogicalVolume()));
    lv_remove_action->setText( i18n("Remove logical volume...") );
    lv_rename_action = lv_actions->addAction("lvrename", this, SLOT(renameLogicalVolume()));
    lv_rename_action->setText( i18n("Rename logical volume..."));
    snap_create_action = lv_actions->addAction("snapcreate", this, SLOT(createSnapshot()));
    snap_create_action->setText( i18n("Create snapshot...") );
    lv_extend_action = lv_actions->addAction( "lvextend", this, SLOT(extendLogicalVolume()));
    lv_extend_action->setText( i18n("Extend logical volume...") );
    pv_move_action = lv_actions->addAction("pvmove", this, SLOT(movePhysicalExtents()));
    pv_move_action->setText( i18n("Move physical extents...") );
    lv_change_action = lv_actions->addAction("lvchange", this, SLOT(changeLogicalVolume()));
    lv_change_action->setText( i18n("Change attributes or tags...")); 

    lv_mkfs_action   = new KAction( i18n("Make filesystem..."), this);
    mount_filesystem_action   = new KAction( i18n("Mount filesystem..."), this);
    unmount_filesystem_action = new KAction( i18n("Unmount filesystem..."), this);
    add_mirror_action  = new KAction( i18n("Add leg or change mirror..."), this);
    remove_mirror_action      = new KAction( i18n("Remove mirror leg..."), this);
    remove_mirror_leg_action  = new KAction( i18n("Remove this mirror leg..."), this);

    connect(lv_mkfs_action, SIGNAL(triggered()), 
	    this, SLOT(mkfsLogicalVolume()));

    connect(add_mirror_action, SIGNAL(triggered()), 
	    this, SLOT(addMirror()));

    connect(mount_filesystem_action, SIGNAL(triggered()), 
	    this, SLOT(mountFilesystem()));

    connect(unmount_filesystem_action, SIGNAL(triggered()), 
	    this, SLOT(unmountFilesystem()));

    connect(remove_mirror_action, SIGNAL(triggered()), 
	    this, SLOT(removeMirror()));

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

    if( m_lv ){
	if(  m_lv->isWritable()  && !m_lv->isLocked() && !m_lv->isVirtual() && 
	    !m_lv->isMirrorLeg() && !m_lv->isMirrorLog() ){

	    if( m_lv->isMounted() ){
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

            if( m_lv->isSnap() || m_lv->isOrigin() ){
                add_mirror_action->setEnabled(false);
		remove_mirror_action->setEnabled(false);

		if( m_lv->isOrigin() && m_lv->isActive() )
		  lv_extend_action->setEnabled(false);
		else
		  lv_extend_action->setEnabled(true);

		lv_reduce_action->setEnabled(false);
                pv_move_action->setEnabled(false);

                if( m_lv->isSnap() ){
                    snap_create_action->setEnabled(false);
                    lv_extend_action->setEnabled(true);
                    lv_reduce_action->setEnabled(true);
                }
                else
                    snap_create_action->setEnabled(true);

                mirror_menu->setEnabled(false);
            }
            else if( m_lv->isMirror() ){
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
        else if( m_lv->isOrphan() ){
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
	else if( m_lv->isPvmove() ){
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
	else if( m_lv->isMirrorLeg() || m_lv->isMirrorLog() ){
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

	    if( !m_lv->isMirrorLog() )
		remove_mirror_leg_action->setEnabled(true);
	    else{
		remove_mirror_leg_action->setEnabled(false);
                mirror_menu->setEnabled(false);
            }
	    snap_create_action->setEnabled(false);
	    filesystem_menu->setEnabled(false);
	}
	else if( !(m_lv->isWritable()) && m_lv->isLocked() ){

	    if( m_lv->isMounted() )
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
	else if( m_lv->isWritable() && m_lv->isLocked() ){

	    if( m_lv->isMounted() )
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
	    if( m_lv->isMounted() ){
		lv_remove_action->setEnabled(false);
		unmount_filesystem_action->setEnabled(true);
	    }
	    else{
		lv_remove_action->setEnabled(true);
		unmount_filesystem_action->setEnabled(false);
	    }

            if( m_lv->isSnap() || m_lv->isOrigin() ){
                add_mirror_action->setEnabled(false);
		remove_mirror_action->setEnabled(false);
                pv_move_action->setEnabled(false);

                if( m_lv->isSnap() )
                    snap_create_action->setEnabled(false);
                else
                    snap_create_action->setEnabled(true);

                mirror_menu->setEnabled(false);
            }
            else if( m_lv->isMirror() ){
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

        if( !m_lv->isActive() ){
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

void LVActionsMenu::createLogicalVolume()
{
    if( lv_create(m_vg) )
	MainWindow->reRun();
} 

void LVActionsMenu::reduceLogicalVolume()
{
    if(lv_reduce(m_lv))
	MainWindow->reRun();
}

void LVActionsMenu::extendLogicalVolume()
{
    if(lv_extend(m_lv))
	MainWindow->reRun();
}

void LVActionsMenu::addMirror()
{
    if( add_mirror(m_lv) )
	MainWindow->reRun();
}

void LVActionsMenu::removeMirror()
{
    if( remove_mirror(m_lv) )
	MainWindow->reRun();
}

void LVActionsMenu::mkfsLogicalVolume()
{
    if( make_fs(m_lv) )
	MainWindow->reRun();
}

void LVActionsMenu::removeLogicalVolume()
{
    if( remove_lv(m_lv) )
	MainWindow->reRun();
}

void LVActionsMenu::renameLogicalVolume()
{
    if( rename_lv(m_lv) )
	MainWindow->reRun();
}

void LVActionsMenu::createSnapshot()
{
    if(snapshot_create(m_lv))
	MainWindow->reRun();
}

void LVActionsMenu::changeLogicalVolume()
{
    if( change_lv(m_lv) )
	MainWindow->reRun();
}

void LVActionsMenu::mountFilesystem()
{
    if( mount_filesystem(m_lv) )
	MainWindow->reRun();
}

void LVActionsMenu::unmountFilesystem()
{
    if( unmount_filesystem(m_lv) )
	MainWindow->reRun();
}

void LVActionsMenu::movePhysicalExtents()
{
    if( move_pv(m_lv) )
        MainWindow->reRun();
}
