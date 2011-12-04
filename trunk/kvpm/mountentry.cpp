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


#include "mountentry.h"

#include <mntent.h>
#include <stdio.h>

#include <QtGui>

#include "logvol.h"
#include "volgroup.h"

// MountEntry provides a thin wrapper for 'mntent' structs by providing
// QStrings rather than const char arrays and a 'mount position' property

MountEntry::MountEntry(mntent *const entry, QObject *parent) : QObject(parent)
{
    m_device_name     = QString( entry->mnt_fsname );
    m_mount_point     = QString( entry->mnt_dir );
    m_filesystem_type = QString( entry->mnt_type );
    m_mount_options   = QString( entry->mnt_opts );

    m_dump_frequency = entry->mnt_freq;
    m_dump_passno    = entry->mnt_passno;
    m_mount_position = 0;
}

MountEntry::MountEntry(mntent *const entry, const QList<mntent *> table, QObject *parent) : QObject(parent)
{
    QStringList mounted_devices;

    m_device_name     = QString( entry->mnt_fsname );
    m_mount_point     = QString( entry->mnt_dir );
    m_filesystem_type = QString( entry->mnt_type );
    m_mount_options   = QString( entry->mnt_opts );

    m_dump_frequency = entry->mnt_freq;
    m_dump_passno    = entry->mnt_passno;
    m_mount_position = 0;

    mounted_devices = getMountedDevices(m_mount_point, table);
    if( mounted_devices.size() > 1 ){
        m_mount_position = 1;
        for( int x = mounted_devices.size() - 1; x >= 0; x-- ){
	    if( m_device_name == mounted_devices[x] )
	        break;
	    else
	        m_mount_position++;
	}
    }
    else
        m_mount_position = 0;
}

/* This function looks at the mount table and returns the
   filesystem's mount point if the device is has an entry
   in the table. */

QStringList MountEntry::getMountedDevices(const QString mountPoint,  const QList<mntent *> table)
{
    mntent *mount_entry;
    QStringList mounted_devices;
    QListIterator<mntent *> entry_itr(table);

    while( entry_itr.hasNext() ){
        mount_entry = entry_itr.next();

        if( QString( mount_entry->mnt_dir ) == mountPoint )
            mounted_devices.append( QString( mount_entry->mnt_fsname ) );
    }

    return mounted_devices;
}

QString MountEntry::getDeviceName()
{
    return m_device_name;
}

QString MountEntry::getMountPoint()
{
    return m_mount_point;
}

QString MountEntry::getFilesystemType()
{
    return m_filesystem_type;
}

QString MountEntry::getMountOptions()
{
    return m_mount_options;
}

int MountEntry::getDumpFrequency()
{
    return m_dump_frequency;
}

int MountEntry::getDumpPassNumber()
{
    return m_dump_passno;
}

int MountEntry::getMountPosition()
{
    return m_mount_position;
}
