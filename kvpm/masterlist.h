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
#ifndef MASTERLIST_H
#define MASTERLIST_H

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
    QList<VolGroup *> VolGroups;
    QList<LogVol *>   LogVols;
    QList<PhysVol *>  PhysVols;
    QList<StorageDevice *> StorageDevices;
    MountInformationList *mount_info_list;
    
    void scanVolGroups();
    VolGroup* scanVolGroups(QString VolumeName);
    void scanLogVols(VolGroup *VolumeGroup);
    void scanLogVols();
    void scanPhysVols(VolGroup *VolumeGroup);
    void scanPhysVols();
    void scanStorageDevices();
    
public:
    MasterList();
    ~MasterList();
    void rebuildVolumeGroup(VolGroup *VolumeGroup);
    const QList<VolGroup *> getVolGroups();
    const QList<PhysVol *> getPhysVols();
    const QList<StorageDevice *> getStorageDevices();
    int getVolGroupCount();
    int getPhysVolCount();
    PhysVol *getPhysVolByName(QString name);
    VolGroup *getVolGroupByName(QString name);
    QStringList getVolumeGroupNames();
    
};

#endif
