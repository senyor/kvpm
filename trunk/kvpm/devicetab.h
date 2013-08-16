/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
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

#include <KMainWindow>

class QSplitter;
class QScrollArea;
class QTreeWidgetItem;
class QVBoxLayout;

class KToolBar;

class DeviceActions;
class DeviceTree;
class DevicePropertiesStack;
class DeviceSizeChart;
class StorageDevice;


class DeviceTab : public KMainWindow
{
    Q_OBJECT

    QVBoxLayout *m_layout = nullptr;
    DeviceTree  *m_tree = nullptr;
    QSplitter   *m_tree_properties_splitter = nullptr;
    DeviceActions *m_device_actions = nullptr; 
    DeviceSizeChart *m_size_chart = nullptr;
    DevicePropertiesStack *m_device_stack = nullptr;

    QScrollArea *setupPropertyStack();
    KToolBar *buildDeviceToolBar();
    void readConfig();

public:
    DeviceTab(QWidget *parent = nullptr);
    void rescan(QList<StorageDevice *> Devices);

private slots:
    void deviceContextMenu(QTreeWidgetItem *item);
};


#endif
