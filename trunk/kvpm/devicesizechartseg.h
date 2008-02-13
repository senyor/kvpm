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
#ifndef STORAGEDEVICESIZECHARTSEG_H
#define STORAGEDEVICESIZECHARTSEG_H

#include <QMenu>
#include <QFrame>

#include "devicemodel.h"
#include "storagepartition.h"

class DeviceChartSeg : public QWidget
{
Q_OBJECT
    QMenu *menu, *vgextend_menu;
    StorageDeviceItem *item;
    QAction *mkfs_action, *pvcreate_action, *pvremove_action,
	    *vgcreate_action, *vgreduce_action, 
	    *mount_action, *unmount_action;
    QList<QAction *> vgextend_actions;
    QString path, use;
    StoragePartition *part;

public:
    DeviceChartSeg(StorageDeviceItem *device_item, QWidget *parent = 0);

public slots:
    void popupContextMenu(QPoint point);

private slots:
    void mkfsPartition();
    void pvcreatePartition();
    void pvremovePartition();
    void vgcreatePartition();
    void vgreducePartition();
    void vgextendPartition(QAction *action);
    
};
	
#endif
