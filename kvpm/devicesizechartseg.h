/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013, 2016 Benjamin Scott   <benscott@nwlink.com>
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

#include <QFrame>

class QTreeWidgetItem;

class StoragePartition;


class DeviceChartSeg : public QFrame
{
    Q_OBJECT

    QTreeWidgetItem *m_item;
    StoragePartition *m_partition;

public:
    explicit DeviceChartSeg(QTreeWidgetItem *const storageItem, QWidget *parent = nullptr);

public slots:
    void popupContextMenu();

signals:
    void deviceMenuRequested(QTreeWidgetItem *item);

};

#endif
