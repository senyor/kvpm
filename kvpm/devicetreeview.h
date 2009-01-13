/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
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

#include "devicemodel.h"

class StoragePartition;

class DeviceTreeView : public QTreeView 
{
Q_OBJECT

    KMenu *menu;
    KMenu *vgextend_menu;

    KAction *mkfs_action, *pvcreate_action, *pvremove_action,
	    *vgcreate_action, *vgreduce_action,
            *mount_action, *unmount_action, 
            *partremove_action, *partadd_action;

    QList<QAction *> vgextend_actions;
    QModelIndex index;
    StorageDeviceItem *item;
    StoragePartition *part;
    
public:
    DeviceTreeView(QWidget *parent = 0);

public slots:
    void popupContextMenu(QPoint point);

private slots:
    void mkfsPartition();
    void pvcreatePartition();
    void pvremovePartition();
    void addPartition();
    void removePartition();
    void vgcreatePartition();
    void vgreducePartition();
    void vgextendPartition(QAction *action);
    void mountPartition();
    void unmountPartition();
    
};

#endif
