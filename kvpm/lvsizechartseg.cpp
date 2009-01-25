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


#include <KMenu>
#include <KAction>
#include <KLocale>
#include <QtGui>

#include "addmirror.h"
#include "logvol.h"
#include "lvactionsmenu.h"
#include "lvsizechartseg.h"
#include "lvcreate.h"
#include "lvchange.h"
#include "lvreduce.h"
#include "lvremove.h"
#include "lvrename.h"
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

LVChartSeg::LVChartSeg(VolGroup *volumeGroup, LogVol *logicalVolume, 
		       QString use, QWidget *parent) : 
    QFrame(parent), 
    m_vg(volumeGroup),
    m_lv(logicalVolume)
{
    
    setFrameStyle( QFrame::Sunken | QFrame::Panel );
    setLineWidth(2);

    QVBoxLayout *layout = new QVBoxLayout();
    QWidget *color_widget = new QWidget();
    layout->addWidget(color_widget);
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);

    QPalette *colorset = new QPalette();

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
    else
	colorset->setColor(QPalette::Window, Qt::black);

    color_widget->setPalette(*colorset);
    color_widget->setAutoFillBackground(true);

    if( !m_vg->isExported() ){
	
	setContextMenuPolicy(Qt::CustomContextMenu);
	m_context_menu = new LVActionsMenu(m_lv, this, this);

	if( m_lv )
	    setToolTip( m_lv->getName() );
	else
	    setToolTip( i18n("free space") );

	connect(this, SIGNAL(customContextMenuRequested(QPoint)), 
		this, SLOT(popupContextMenu(QPoint)) );
    }
}

void LVChartSeg::popupContextMenu(QPoint)
{
    m_context_menu->exec(QCursor::pos());
}


void LVChartSeg::extendLogicalVolume()
{
    if(lv_extend(m_lv))
	MainWindow->reRun();
}

void LVChartSeg::createLogicalVolume()
{
    if(lv_create(m_vg))
	MainWindow->reRun();
}

void LVChartSeg::reduceLogicalVolume()
{
    if(lv_reduce(m_lv))
	MainWindow->reRun();
}

void LVChartSeg::addMirror()
{
    if( add_mirror(m_lv) )
	MainWindow->reRun();
}

void LVChartSeg::removeMirror()
{
    if( remove_mirror(m_lv) )
	MainWindow->reRun();
}

void LVChartSeg::mkfsLogicalVolume()
{
    if( make_fs(m_lv) )
	MainWindow->rebuildVolumeGroupTab();
}

void LVChartSeg::removeLogicalVolume()
{
    if( remove_lv(m_lv) )
	MainWindow->reRun();
}

void LVChartSeg::renameLogicalVolume()
{
    if( rename_lv(m_lv) )
	MainWindow->rebuildVolumeGroupTab();
}

void LVChartSeg::createSnapshot()
{
    if(snapshot_create(m_lv))
	MainWindow->reRun();
}

void LVChartSeg::changeLogicalVolume()
{
    if( change_lv(m_lv) )
	MainWindow->rebuildVolumeGroupTab();
}

void LVChartSeg::mountFilesystem()
{
    if( mount_filesystem(m_lv) )
	MainWindow->reRun();
}

void LVChartSeg::unmountFilesystem()
{
    if( unmount_filesystem(m_lv) )
	MainWindow->reRun();
}

void LVChartSeg::movePhysicalExtents()
{
    if( move_pv(m_lv) )
        MainWindow->reRun();
}
