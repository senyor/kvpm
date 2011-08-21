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

#ifndef STORAGEDEVICESIZECHART_H
#define STORAGEDEVICESIZECHART_H

#include <QAbstractItemModel>
#include <QList>
#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>

#include "devicemodel.h"
#include "devicesizechartseg.h"

class DeviceSizeChart : public QFrame
{
    Q_OBJECT

    QHBoxLayout *m_layout;
    QHBoxLayout *m_extended_layout;        // The layout for chart segments inside an extended partition

    QList<QWidget *> m_segments, m_extended_segments;    // Segments of the bar chart, not the disk.
    QList<double> m_ratios, m_extended_ratios;
    
    StorageDeviceModel *m_device_model;

public:
    DeviceSizeChart(StorageDeviceModel *model, QWidget *parent);
    void resizeEvent(QResizeEvent *event);
    
public slots:
     void setNewDevice(QModelIndex index);
};

#endif
