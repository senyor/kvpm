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

#ifndef STORAGEDEVICESIZECHARTSEG_H
#define STORAGEDEVICESIZECHARTSEG_H

class StoragePartition;
class StorageDeviceItem;


#include <QMenu>
#include <QFrame>


class DeviceChartSeg : public QWidget
{
Q_OBJECT

    QMenu *m_context_menu, 
          *m_vgextend_menu;

    StorageDeviceItem *m_item;

    QAction *m_mkfs_action, 
	    *m_pvcreate_action, 
            *m_pvremove_action,
            *m_vgcreate_action, 
            *m_vgreduce_action, 
            *m_mount_action, 
            *m_unmount_action;

    QString m_pv_name;
    
    StoragePartition *m_partition;

public:
    DeviceChartSeg(StorageDeviceItem *storageDeviceItem, QWidget *parent = 0);

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
