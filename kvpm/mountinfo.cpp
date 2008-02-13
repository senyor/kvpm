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


#include <mntent.h>
#include <stdio.h>
#include <QtGui>
#include "logvol.h"
#include "mountinfo.h"
#include "volgroup.h"


MountInformationList::MountInformationList()
{
    const char mount_table[] = _PATH_MOUNTED;
    mntent *mount_entry;
    
    FILE *fp = setmntent(mount_table, "r");
    if(fp){
	while( (mount_entry = getmntent(fp)) )
	    list.append( new MountInformation(mount_entry) );

	endmntent(fp);
    }
}

MountInformationList::~MountInformationList()
{
    for(int x = 0; x < list.size(); x++)
	delete (list[x]);
}

QList<MountInformation *> MountInformationList::getMountInformation(QString DeviceName)
{
    QList<MountInformation *> device_mounts;
    
    for(int x = list.size() -1; x >= 0; x--){
	if( DeviceName == list[x]->getDeviceName() )
	    device_mounts.append( list.takeAt(x) );
    }
    return device_mounts;
}

MountInformation::MountInformation(mntent *MountTableEntry, QObject *parent) : QObject(parent)
{
    device_name     = QString( MountTableEntry->mnt_fsname );
    mount_point     = QString( MountTableEntry->mnt_dir );
    filesystem_type = QString( MountTableEntry->mnt_type );
    mount_options   = QString( MountTableEntry->mnt_opts );

    dump_frequency = MountTableEntry->mnt_freq;
    dump_passno    = MountTableEntry->mnt_passno;
}

QString MountInformation::getDeviceName()
{
    return device_name;
}

QString MountInformation::getMountPoint()
{
    return mount_point;
}

QString MountInformation::getFilesystemType()
{
    return filesystem_type;
}

QString MountInformation::getMountOptions()
{
    return mount_options;
}

int MountInformation::getDumpFrequency()
{
    return dump_frequency;
}

int MountInformation::getDumpPassNumber()
{
    return dump_passno;
}

