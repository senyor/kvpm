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
#ifndef STORAGEDEVICESIZECHART_H
#define STORAGEDEVICESIZECHART_H

#include <QAbstractItemModel>
#include <QList>
#include <QLabel>
#include <QFrame>

#include "devicemodel.h"
#include "devicesizechartseg.h"

class DeviceSizeChart : public QFrame
{
    Q_OBJECT

    QHBoxLayout *layout;
    QHBoxLayout *extended_layout;        // The layout for chart segments inside an extented partition

    QList<QWidget *> segments, extended_segments;    // Segments of the bar chart, not the disk.
    QList<double> ratios, extended_ratios;
    
    StorageDeviceModel *device_model;

public:
    DeviceSizeChart(StorageDeviceModel *model, QWidget *parent);
    void resizeEvent(QResizeEvent *event);
    
public slots:
     void setNewDevice(QModelIndex index);
};

#endif
