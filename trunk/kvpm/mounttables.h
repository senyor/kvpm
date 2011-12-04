/*
 *
 * 
 * Copyright (C) 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef MOUNTTABLES_H
#define MOUNTTABLES_H

#include <QString>
#include <QList>

class LogVol;
class StoragePartition;

#ifndef MOUNTINFO_H
class MountInformation;
#endif

class MountTables
{
    QList<MountInformation *> m_list;
    QList<MountInformation *> m_fstab_list;
    QString getFstabMountPoint(const QString name, const QString label, const QString uuid);
    
 public:
    MountTables();
    ~MountTables();
    QList<MountInformation *> getMountInformation(const QString deviceName);
    QString getFstabMountPoint(LogVol *const lv);
    QString getFstabMountPoint(StoragePartition *const partition);

};

#endif
