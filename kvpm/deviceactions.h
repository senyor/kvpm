/*
 *
 *
 * Copyright (C) 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#ifndef DEVICEACTIONS_H
#define DEVICEACTIONS_H

#include <KActionCollection>

#include <QAction>


class QActionGroup;
class QTreeWidgetItem;
class StoragePartition;
class StorageDevice;



class DeviceActions : public KActionCollection
{
    Q_OBJECT

    StoragePartition *m_part = nullptr;
    StorageDevice *m_dev = nullptr;
    QActionGroup *m_act_grp = nullptr;

    void vgextendEnable(bool enable);

public:
    explicit DeviceActions(QWidget *parent = nullptr);

public slots:
    void changeDevice(QTreeWidgetItem *item);

private slots:
    void extendVg(QAction *action);
    void checkFs();
    void makeFs();
    void maxFs();
    void addPartition();
    void changePartition();
    void changeFlags();
    void removePartition();
    void createVg();
    void createTable();
    void reduceVg();
    void mountFs();
    void unmountFs();

};

#endif
