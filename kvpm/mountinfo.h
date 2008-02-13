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
#ifndef MOUNTINFO_H
#define MOUNTINFO_H

#include <QString>
#include <QList>

class mntent;
class MountInformation;


class MountInformationList
{
    QList<MountInformation *> list;
    
 public:
    MountInformationList();
    ~MountInformationList();
    QList<MountInformation *> getMountInformation(QString DeviceName);
};

    
class MountInformation : public QObject
{
    QString device_name,        // for example: "/dev/sda1"
	    mount_point, 
	    filesystem_type,    // ext3, reiserfs, swap etcetera 
	    mount_options;      // options, such as "noatime," set when mounting a filesystem 

    int dump_frequency; 
    int dump_passno;

 public:
    MountInformation(mntent *MountTableEntry, QObject *parent = 0);
    QString getDeviceName();
    QString getMountPoint();
    QString getFilesystemType();
    QString getMountOptions();
    int getDumpFrequency();
    int getDumpPassNumber();
    
};

#endif
