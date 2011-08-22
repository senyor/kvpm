/*
 *
 * 
 * Copyright (C) 2008, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "mountinfo.h"

#include <mntent.h>
#include <stdio.h>

#include <QtGui>

#include "logvol.h"
#include "mountentry.h"
#include "volgroup.h"


MountInformation::MountInformation(mntent *mountTableEntry, QObject *parent) : QObject(parent)
{

    QStringList mounted_devices;

    m_device_name     = QString( mountTableEntry->mnt_fsname );
    m_mount_point     = QString( mountTableEntry->mnt_dir );
    m_filesystem_type = QString( mountTableEntry->mnt_type );
    m_mount_options   = QString( mountTableEntry->mnt_opts );

    m_dump_frequency = mountTableEntry->mnt_freq;
    m_dump_passno    = mountTableEntry->mnt_passno;
    m_mount_position = 0;

    mounted_devices = getMountedDevices(m_mount_point);
    if( mounted_devices.size() > 1 ){
        m_mount_position = 1;
        for( int x = mounted_devices.size() - 1; x >= 0; x-- ){
	    if( m_device_name == mounted_devices[x] )
	        break;
	    else
	        m_mount_position++;
	}
    }
    else{
        m_mount_position = 0;
    }

}

QString MountInformation::getDeviceName()
{
    return m_device_name;
}

QString MountInformation::getMountPoint()
{
    return m_mount_point;
}

QString MountInformation::getFilesystemType()
{
    return m_filesystem_type;
}

QString MountInformation::getMountOptions()
{
    return m_mount_options;
}

int MountInformation::getDumpFrequency()
{
    return m_dump_frequency;
}

int MountInformation::getDumpPassNumber()
{
    return m_dump_passno;
}

int MountInformation::getMountPosition()
{
    return m_mount_position;
}
