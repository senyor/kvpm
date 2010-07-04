/*
 *
 * 
 * Copyright (C) 2008, 2009 Benjamin Scott   <benscott@nwlink.com>
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
#include <QTextEdit>
#include <QProcess>
#include <QStringList>

class VolGroup;
class LogVol;
class PhysVol;
class MountInformation;
class MountInformationList;
class StorageDevice;


class MasterList : public QObject
{
    QList<VolGroup *> m_volume_groups;
    QList<LogVol *>   m_logical_volumes;
    QList<StorageDevice *> m_storage_devices;
    
    void scanVolumeGroups(lvm_t lvm);
    void scanLogicalVolumes(VolGroup *VolumeGroup);
    void scanLogicalVolumes();
    void scanStorageDevices();
    bool determinePVState(PhysVol *pv, VolGroup *vg); 

public:
    MasterList();
    ~MasterList();
    const QList<VolGroup *> getVolGroups();
    const QList<StorageDevice *> getStorageDevices();
    int getVolGroupCount();
    VolGroup *getVolGroupByName(QString name);
    QStringList getVolumeGroupNames();
    
};

#endif
