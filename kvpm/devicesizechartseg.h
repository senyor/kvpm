/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
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

#include <QMenu>
#include <QFrame>


class StoragePartition;
class StorageItem;


class DeviceChartSeg : public QFrame
{
Q_OBJECT

    StorageItem *m_item;
    StoragePartition *m_partition;

public:
    explicit DeviceChartSeg(StorageItem *storageItem, QWidget *parent = 0);

public slots:
    void popupContextMenu(QPoint point);

};
	
#endif
