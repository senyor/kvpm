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

#ifndef MOUNTTABLES_H
#define MOUNTTABLES_H

#include <QString>
#include <QList>

class LogVol;
class StoragePartition;
class MountEntry;


class MountTables
{
    QList<MountEntry *> m_mount_list;
    QList<MountEntry *> m_fstab_list;

    QString getFstabMountPoint(const QString name, const QString label, const QString uuid);
    
 public:
    MountTables();
    ~MountTables();

    void loadData();
    QList<MountEntry *> getMtabEntries(const int major, const int minor); // The list entries must be freed/deleted by the reciever
    QString getFstabMountPoint(LogVol *const lv);
    QString getFstabMountPoint(StoragePartition *const partition);

    static bool addEntry(const QString device, const QString mountPoint, const QString type, 
                              const QString options, const int dumpFreq, const int pass);
    static bool renameEntries(const QString oldName, const QString newName);
    static bool removeEntry(const QString mountPoint);
};

#endif
