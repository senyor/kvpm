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


#include <KMenu>
#include <KAction>

#include <QtGui>
#include "addmirror.h"
#include "logvol.h"
#include "lvactionsmenu.h"
#include "lvsizechartseg.h"
#include "lvcreate.h"
#include "lvchange.h"
#include "lvreduce.h"
#include "lvremove.h"
#include "mkfs.h"
#include "mount.h"
#include "processprogress.h"
#include "pvmove.h"
#include "removemirror.h"
#include "topwindow.h"
#include "unmount.h"
#include "volgroup.h"



/* This should be passed *lv = 0 if it is really free space on a volume
   group that is being displayed */

LVChartSeg::LVChartSeg(VolGroup *VolumeGroup, LogVol *LogicalVolume, 
		       QString use, QWidget *parent) : 
    QWidget(parent), 
    vg(VolumeGroup),
    lv(LogicalVolume)
{
    
    QPalette *colorset = new QPalette();
    if(use == "ext2")
	colorset->setColor(QPalette::Window, Qt::blue);
    if(use == "ext3")
	colorset->setColor(QPalette::Window, Qt::darkBlue);
    if(use == "reiserfs")
	colorset->setColor(QPalette::Window, Qt::red);
    if(use == "reiser4")
	colorset->setColor(QPalette::Window, Qt::darkGray);
    if(use == "hfs")
	colorset->setColor(QPalette::Window, Qt::darkRed);
    if(use == "vfat")
	colorset->setColor(QPalette::Window, Qt::yellow);
    if(use == "jfs")
	colorset->setColor(QPalette::Window, Qt::magenta);
    if(use == "xfs")
	colorset->setColor(QPalette::Window, Qt::cyan);
    if(use == "linux-swap")
	colorset->setColor(QPalette::Window, Qt::lightGray);
    if(use == "")
	colorset->setColor(QPalette::Window, Qt::black);
    if(use == "freespace")
	colorset->setColor(QPalette::Window, Qt::green);
    setPalette(*colorset);
    setAutoFillBackground(TRUE);

    setContextMenuPolicy(Qt::CustomContextMenu);
    context_menu = new LVActionsMenu(lv, this, this);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), 
	    this, SLOT(popupContextMenu(QPoint)) );
}

void LVChartSeg::popupContextMenu(QPoint)
{
    context_menu->exec(QCursor::pos());
}


void LVChartSeg::extendLogicalVolume()
{
    if(LVExtend(lv))
	MainWindow->reRun();
}

void LVChartSeg::createLogicalVolume()
{
    if(LVCreate(vg))
	MainWindow->reRun();
}

void LVChartSeg::reduceLogicalVolume()
{
    if(LVReduce(lv))
	MainWindow->reRun();
}

void LVChartSeg::addMirror()
{
    if( add_mirror(lv) )
	MainWindow->reRun();
}

void LVChartSeg::removeMirror()
{
    if( remove_mirror(lv) )
	MainWindow->reRun();
}

void LVChartSeg::mkfsLogicalVolume()
{
    if( make_fs(lv) )
	MainWindow->rebuildVolumeGroupTab(vg);
}

void LVChartSeg::removeLogicalVolume()
{
    if( remove_lv(lv) )
	MainWindow->reRun();
}

void LVChartSeg::createSnapshot()
{
    if(SnapshotCreate(lv))
	MainWindow->reRun();
}

void LVChartSeg::changeLogicalVolume()
{
    if( change_lv(lv) )
	MainWindow->rebuildVolumeGroupTab(vg);
}

void LVChartSeg::mountFilesystem()
{
    if( mount_filesystem(lv) )
	MainWindow->rebuildVolumeGroupTab(vg);
}

void LVChartSeg::unmountFilesystem()
{
    if( unmount_filesystem(lv) )
	MainWindow->rebuildVolumeGroupTab(vg);
}

void LVChartSeg::movePhysicalExtents()
{
    if( move_pv(lv) )
        MainWindow->reRun();
}
