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


#include "devicetreeview.h"

#include <QtGui>
#include <KLocale>

#include "deviceactionsmenu.h"


DeviceTreeView::DeviceTreeView(QWidget *parent) : QTreeView(parent)
{

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), 
	    this, SLOT(popupContextMenu(QPoint)) );

}

void DeviceTreeView::popupContextMenu(QPoint point)
{
    KMenu *context_menu;

    index = indexAt(point);

    //item = 0 if there is no item a that point

    item = static_cast<StorageDeviceItem*> (index.internalPointer());

    if(item){                          
        if( (item->dataAlternate(0)).canConvert<void *>() )
            part = (StoragePartition *) (( item->dataAlternate(0)).value<void *>() );
        context_menu = new DeviceActionsMenu(item, this);
        context_menu->exec(QCursor::pos());
    }
}

