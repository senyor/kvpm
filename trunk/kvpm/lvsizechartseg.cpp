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


#include "lvsizechartseg.h"

#include <KConfigSkeleton>
#include <KMenu>
#include <KLocale>
#include <QtGui>

#include "logvol.h"
#include "lvactionsmenu.h"
#include "volgroup.h"



/* This should be passed *lv = 0 if it is really free space on a volume
   group that is being displayed */

LVChartSeg::LVChartSeg(VolGroup *volumeGroup, LogVol *logicalVolume, QString use, QWidget *parent) : 
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

    KConfigSkeleton skeleton;

    QColor  ext2_color,   ext3_color,    ext4_color,
            reiser_color, reiser4_color, msdos_color,
            jfs_color,    xfs_color,     none_color,
            free_color,   swap_color,    hfs_color,
            btrfs_color,  ntfs_color;

    skeleton.setCurrentGroup("FilesystemColors");
    skeleton.addItemColor("ext2",    ext2_color,  Qt::blue);
    skeleton.addItemColor("ext3",    ext3_color,  Qt::darkBlue);
    skeleton.addItemColor("ext4",    ext4_color,  Qt::cyan);
    skeleton.addItemColor("btrfs",   btrfs_color,   Qt::yellow);
    skeleton.addItemColor("reiser",  reiser_color,  Qt::red);
    skeleton.addItemColor("reiser4", reiser4_color, Qt::darkRed);
    skeleton.addItemColor("msdos",   msdos_color, Qt::darkYellow);
    skeleton.addItemColor("jfs",     jfs_color,   Qt::magenta);
    skeleton.addItemColor("xfs",     xfs_color,   Qt::darkCyan);
    skeleton.addItemColor("hfs",     hfs_color,   Qt::darkMagenta);
    skeleton.addItemColor("ntfs",    ntfs_color,  Qt::darkGray);
    skeleton.addItemColor("none",    none_color,  Qt::black);
    skeleton.addItemColor("free",    free_color,  Qt::green);
    skeleton.addItemColor("swap",    swap_color,  Qt::lightGray);

    if(use == "ext2")
	colorset->setColor(QPalette::Window, ext2_color);
    else if(use == "ext3")
	colorset->setColor(QPalette::Window, ext3_color);
    else if(use == "ext4")
	colorset->setColor(QPalette::Window, ext4_color);
    else if(use == "btrfs")
	colorset->setColor(QPalette::Window, btrfs_color);
    else if(use == "reiserfs")
	colorset->setColor(QPalette::Window, reiser_color);
    else if(use == "reiser4")
	colorset->setColor(QPalette::Window, reiser4_color);
    else if(use == "hfs")
	colorset->setColor(QPalette::Window, hfs_color);
    else if(use == "vfat")
	colorset->setColor(QPalette::Window, msdos_color);
    else if(use == "jfs")
        colorset->setColor(QPalette::Window, jfs_color);
    else if(use == "xfs")
	colorset->setColor(QPalette::Window, xfs_color);
    else if(use == "ntfs")
	colorset->setColor(QPalette::Window, ntfs_color);
    else if(use == "swap")
	colorset->setColor(QPalette::Window, swap_color);
    else if(use == "freespace")
	colorset->setColor(QPalette::Window, free_color);
    else
	colorset->setColor(QPalette::Window, none_color);

    color_widget->setPalette(*colorset);
    color_widget->setAutoFillBackground(true);

    if( !m_vg->isExported() ){
	
	setContextMenuPolicy(Qt::CustomContextMenu);

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
    m_context_menu = new LVActionsMenu(m_lv, -1, m_vg, this);
    m_context_menu->exec(QCursor::pos());
    m_context_menu->deleteLater();
}
