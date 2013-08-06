/*
 *
 *
 * Copyright (C) 2008, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "devicepropertiesstack.h"

#include "deviceproperties.h"
#include "devicetree.h"
#include "storagedevice.h"
#include "storagepartition.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QTreeWidgetItem>


/* This stack widget simply displays some information about the
   drive or device selected in the tree view. If nothing is selected
   an empty widget is used.  */

DevicePropertiesStack::DevicePropertiesStack(QWidget *parent) : 
    QStackedWidget(parent)
{
    addWidget(getDefaultWidget());
    setCurrentIndex(0);
}

void DevicePropertiesStack::changeDeviceStackIndex(QTreeWidgetItem *item)
{
    setCurrentIndex(0);

    if (!item)
        return;

    const QString device_path = item->data(0, Qt::DisplayRole).toString();
    const int list_size = m_device_path_list.size();

    for (int x = 0; x < list_size; ++x) {
        if (m_device_path_list[x] == device_path)
            setCurrentIndex(x);
    }
}

void DevicePropertiesStack::loadData(QList<StorageDevice *> devices)
{
    QWidget *stackWidget;

    m_device_path_list.clear();

    for (int x = count() - 1; x >= 0; --x) { // delete old member widgets
        stackWidget = widget(x);
        removeWidget(stackWidget);
        delete stackWidget;
    }

    for (auto dev : devices) {
        m_device_path_list.append(dev->getName());
        addWidget(new DeviceProperties(dev));

        for (auto part : dev->getStoragePartitions()) {
            m_device_path_list << part->getName();
            addWidget(new DeviceProperties(part));
        }
    }

    addWidget(getDefaultWidget());
    setCurrentIndex(0);
}

QWidget *DevicePropertiesStack::getDefaultWidget()
{
    QWidget *dw = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout();

    layout->addWidget(new QLabel("Random String For The Layout"));
    dw->setLayout(layout);
    return dw;
}
