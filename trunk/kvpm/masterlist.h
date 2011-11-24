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
#include <QProcess>
#include <QStringList>
#include <QTextEdit>

class VolGroup;
class LogVol;
class PhysVol;
class MountInformation;
class MountInformationList;
class ProgressBox;
class StorageDevice;


class MasterList : public QObject
{
Q_OBJECT

    static QList<VolGroup *> m_volume_groups;
    static QList<StorageDevice *> m_storage_devices;
    static lvm_t m_lvm;

    void scanVolumeGroups();
    void scanStorageDevices();
    //    bool determinePVState(PhysVol *const pv, VolGroup *const vg); 

public:
    MasterList();
    ~MasterList();
    void rescan();    
    static lvm_t getLVM();
    static const QList<VolGroup *> getVolGroups();
    static const QList<StorageDevice *> getStorageDevices();
    static int getVolGroupCount();
    static VolGroup *getVolGroupByName(QString name);
    static QStringList getVolumeGroupNames();
};

#endif
