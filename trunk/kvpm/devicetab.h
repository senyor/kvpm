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
#ifndef DEVICETAB_H
#define DEVICETAB_H

#include <QList>
#include <QWidget>
#include <QSplitter>
					
class StorageDevice;
class MasterList;
class DeviceTreeView;

class DeviceTab : public QWidget
{
    QList<StorageDevice *> devs;
    DeviceTreeView *tree;

    QSplitter *setupPropertyWidgets();

 public:
    DeviceTab(QList<StorageDevice *> Devices, QWidget *parent = 0);

};


#endif
