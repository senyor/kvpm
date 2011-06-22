/*
 *
 * 
 * Copyright (C) 2008, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef DEVICEACTIONSMENU_H
#define DEVICEACTIONSMENU_H

#include <KMenu>
#include <KAction>

#include <QTreeView>
#include <QAbstractItemModel>
#include <QString>

#include "devicemodel.h"

class StoragePartition;
class StorageDevice;
class DeviceTreeView;
class DeviceChartSeg;

class DeviceActionsMenu : public KMenu
{
Q_OBJECT

    KMenu *m_vgextend_menu;

    KAction *m_mkfs_action,
            *m_maxfs_action,
            *m_maxpv_action,
            *m_partremove_action,
            *m_partadd_action,
            *m_partmoveresize_action,
            *m_removefs_action,
	    *m_vgcreate_action,
	    *m_tablecreate_action,
	    *m_vgreduce_action,
	    *m_mount_action,
            *m_unmount_action;

    QList<QAction *> vgextend_actions;
    StoragePartition *m_part;
    StorageDevice *m_dev;
    QString m_vg_name;

    void setup(StorageDeviceItem *item);
    
 public:
    DeviceActionsMenu( StorageDeviceItem *item, QWidget *parent = 0);

 private slots:
    void mkfsPartition();
    void maxfsPartition();
    void addPartition();
    void moveresizePartition();
    void removePartition();
    void removefsPartition();
    void vgcreatePartition();
    void tablecreatePartition();
    void vgreducePartition();
    void vgextendPartition(QAction *action);
    void mountPartition();
    void unmountPartition();
    

};

#endif
