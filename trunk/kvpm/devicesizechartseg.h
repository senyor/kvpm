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


class DeviceChartSeg : public QFrame
{
Q_OBJECT

    StorageDeviceItem *m_item;

    QString m_pv_name;
    
    StoragePartition *m_partition;

public:
    DeviceChartSeg(StorageDeviceItem *storageDeviceItem, QWidget *parent = 0);

public slots:
    void popupContextMenu(QPoint point);
    void addPartition();
    void mkfsPartition();
    void mountPartition();
    void pvcreatePartition();
    void pvremovePartition();
    void removePartition();
    void moveresizePartition();
    void vgcreatePartition();
    void vgreducePartition();
    void vgextendPartition(QAction *action);
    void unmountPartition();

};
	
#endif
