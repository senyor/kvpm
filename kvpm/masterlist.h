/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef MASTERLIST_H
#define MASTERLIST_H

#include <lvm2app.h>

#include <QList>
#include <QStringList>

class QTextEdit;
class QProcess;

class VolGroup;
class LogVol;
class MountTables;
class PhysVol;
class StorageDevice;


class MasterList : public QObject
{
    Q_OBJECT

    MountTables *m_mount_tables;
    static QList<VolGroup *> m_volume_groups;
    static QList<StorageDevice *> m_storage_devices;
    static lvm_t m_lvm;

    void scanVolumeGroups();
    void scanStorageDevices();

public:
    MasterList();
    ~MasterList();
    void rescan();
    static lvm_t getLvm();
    static QList<VolGroup *> getVolGroups();
    static QList<StorageDevice *> getStorageDevices();
    static int getVgCount();
    static VolGroup *getVgByName(QString name);
    static QStringList getVgNames();
};

#endif
