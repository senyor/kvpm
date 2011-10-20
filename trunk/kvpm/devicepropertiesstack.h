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


#ifndef DEVICEPROPERTIESSTACK_H
#define DEVICEPROPERTIESSTACK_H

#include <QList>
#include <QTreeWidgetItem>
#include <QStackedWidget>
#include <QStringList>

class StorageDevice;


class DevicePropertiesStack : public QStackedWidget
{
Q_OBJECT

    QStringList m_device_path_list;  // full path of each device on the stack, in the same order

    QWidget *getDefaultWidget();

 public:
     explicit DevicePropertiesStack(QWidget *parent = 0);
     void loadData(QList<StorageDevice *> devices);

 public slots:
     void changeDeviceStackIndex(QTreeWidgetItem *item);
};

#endif

