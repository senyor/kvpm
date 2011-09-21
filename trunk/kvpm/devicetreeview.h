/*
 *
 * 
 * Copyright (C) 2008, 2009, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef DEVICETREEVIEW_H
#define DEVICETREEVIEW_H

#include <KMenu>
#include <KAction>

#include <QTreeView>
#include <QAbstractItemModel>

class StoragePartition;
class StorageItem;

class DeviceTreeView : public QTreeView 
{
Q_OBJECT

    QList<QAction *> vgextend_actions;
    QModelIndex index;
    StorageItem *item;
    StoragePartition *part;
    
public:
    DeviceTreeView(QWidget *parent = 0);

public slots:
    void popupContextMenu(QPoint point);

};

#endif
