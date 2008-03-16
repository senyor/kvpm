/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef MOUNTINFO_H
#define MOUNTINFO_H

#include <QString>
#include <QList>

class mntent;
class MountInformation;


class MountInformationList
{
    QList<MountInformation *> m_list;
    
 public:
    MountInformationList();
    ~MountInformationList();
    QList<MountInformation *> getMountInformation(QString deviceName);
};

    
class MountInformation : public QObject
{
    QString m_device_name,        // for example: "/dev/sda1"
	    m_mount_point, 
	    m_filesystem_type,    // ext3, reiserfs, swap etcetera 
	    m_mount_options;      // options, such as "noatime," set when mounting a filesystem 

    int m_dump_frequency; 
    int m_dump_passno;

 public:
    MountInformation(mntent *mountTableEntry, QObject *parent = 0);
    QString getDeviceName();
    QString getMountPoint();
    QString getFilesystemType();
    QString getMountOptions();
    int getDumpFrequency();
    int getDumpPassNumber();
    
};

#endif
