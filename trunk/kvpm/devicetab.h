/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#ifndef DEVICETAB_H
#define DEVICETAB_H

#include <QList>
#include <QWidget>
#include <QSplitter>
#include <QScrollArea>
#include <QVBoxLayout>

class DeviceTree;
class DevicePropertiesStack;
class DeviceSizeChart;
class StorageDevice;

class DeviceTab : public QWidget
{
    QVBoxLayout *m_layout;
    DeviceTree  *m_tree;
    QSplitter   *m_tree_properties_splitter;
    DeviceSizeChart *m_size_chart;
    DevicePropertiesStack *m_device_stack;

    QScrollArea *setupPropertyStack();

public:
    DeviceTab(QWidget *parent = 0);
    void rescan(QList<StorageDevice *> Devices);
};


#endif
