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
#include <KConfigSkeleton>

#include "devicemodel.h"
#include "devicesizechartseg.h"
#include "deviceactionsmenu.h"
#include "storagepartition.h"


DeviceChartSeg::DeviceChartSeg(StorageDeviceItem *storageDeviceItem, QWidget *parent) : 
    QFrame(parent),
    m_item(storageDeviceItem)
{
    QStringList group_names;
    QString use;
    QPalette *colorset = new QPalette();

    KConfigSkeleton skeleton;

    QColor  ext2_color,   ext3_color,    ext4_color,
            reiser_color, reiser4_color, msdos_color,
            jfs_color,    xfs_color,     none_color,
            free_color,   swap_color,    hfs_color,
            physical_color;

    skeleton.setCurrentGroup("FilesystemColors");
    skeleton.addItemColor("ext2",   ext2_color);
    skeleton.addItemColor("ext3",   ext3_color);
    skeleton.addItemColor("ext4",   ext4_color);
    skeleton.addItemColor("reiser",  reiser_color);
    skeleton.addItemColor("reiser4", reiser4_color);
    skeleton.addItemColor("msdos", msdos_color);
    skeleton.addItemColor("jfs",   jfs_color);
    skeleton.addItemColor("xfs",   xfs_color);
    skeleton.addItemColor("none",  none_color);
    skeleton.addItemColor("free",  free_color);
    skeleton.addItemColor("swap",  swap_color);
    skeleton.addItemColor("hfs",   hfs_color);
    skeleton.addItemColor("physvol", physical_color);

    use = (m_item->data(4)).toString();

    m_partition = NULL;
    if( (m_item->dataAlternate(0)).canConvert<void *>() ){
	m_partition = (StoragePartition *) (( m_item->dataAlternate(0)).value<void *>() );

        if ( m_partition->getPedType() & 0x02 ){  // extended
            setFrameStyle(QFrame::Raised | QFrame::Box);
            setLineWidth( 1 );
	    colorset->setColor(QPalette::Window, Qt::green);
        }
        else if( m_partition->getPedType() & 0x04 ){   // freespace
            setFrameStyle(QFrame::Sunken | QFrame::Panel);
            setLineWidth( 2 );
            colorset->setColor(QPalette::Window, Qt::green);
        }
        else{
            setFrameStyle(QFrame::Sunken | QFrame::Panel);
            setLineWidth( 2 );
            
            if(use == "ext2")
                colorset->setColor(QPalette::Window, ext2_color);
            else if(use == "ext3")
                colorset->setColor(QPalette::Window, ext3_color);
            else if(use == "ext4")
                colorset->setColor(QPalette::Window, ext4_color);
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
            else if(use == "swap")
                colorset->setColor(QPalette::Window, swap_color);
            else if(use == "freespace")
                colorset->setColor(QPalette::Window, free_color);
            else if(use == "physical volume")
                colorset->setColor(QPalette::Window, physical_color);
            else
                colorset->setColor(QPalette::Window, none_color);
        }
    }
    else{  // whole device, not a partition
            setFrameStyle( QFrame::Sunken | QFrame::Panel );
            setLineWidth( 2 );
            colorset->setColor(QPalette::Window, none_color);
    }
	
    setPalette(*colorset);
    setAutoFillBackground(true);

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), 
	    this, SLOT(popupContextMenu(QPoint)) );

}

void DeviceChartSeg::popupContextMenu(QPoint point)
{
    (void)point;

    KMenu *context_menu;

    if( (m_item->dataAlternate(0)).canConvert<void *>() )
        m_partition = (StoragePartition *) (( m_item->dataAlternate(0)).value<void *>() );

    if(m_item){  // m_item = 0 if there is no item a that point
        context_menu = new DeviceActionsMenu(m_item, this);
        context_menu->exec(QCursor::pos());
    }
    else{
        context_menu = new DeviceActionsMenu(NULL, this);
        context_menu->exec(QCursor::pos());
    }
}
