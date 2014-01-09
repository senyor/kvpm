/*
 *
 *
 * Copyright (C) 2008, 2011, 2012, 2014 Benjamin Scott   <benscott@nwlink.com>
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

#include <QFrame>
#include <QList>

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHBoxLayout>



class DeviceSizeChart : public QFrame
{
    Q_OBJECT

    QHBoxLayout *m_layout;
    QHBoxLayout *m_extended_layout;        // The layout for chart segments inside an extended partition

    QList<QWidget *> m_segments, m_extended_segments;    // Segments of the bar chart, not the disk.
    QList<double>    m_ratios,   m_extended_ratios;


public:
    DeviceSizeChart(QWidget *parent);
    void resizeEvent(QResizeEvent *event);

public slots:
    void setNewDevice(QTreeWidgetItem *deviceItem);

signals:
    void deviceMenuRequested(QTreeWidgetItem *item);

};

#endif
