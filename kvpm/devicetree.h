/*
 *
 *
 * Copyright (C) 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#ifndef DEVICETREE_H
#define DEVICETREE_H

#include <KMenu>

#include <QPoint>
#include <QList>
#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>


class StorageDevice;
class DeviceSizeChart;
class DevicePropertiesStack;

class DeviceTree : public QTreeWidget
{
    Q_OBJECT

    bool m_initial_run,
         m_show_total,
         m_show_percent,
         m_show_both,
         m_expand_parts,
         m_use_si_units;

    int m_fs_warn_percent,
        m_pv_warn_percent;

    DeviceSizeChart       *m_chart;
    DevicePropertiesStack *m_stack;

    void setupContextMenu();
    void setViewConfig();

public:
    DeviceTree(DeviceSizeChart *const chart, DevicePropertiesStack *const stack, QWidget *parent = NULL);
    void loadData(QList<StorageDevice *> devices);

private slots:
    void popupContextMenu(QPoint point);

signals:
    void deviceMenuRequested(QTreeWidgetItem *);

};

#endif
