/*
 *
 *
 * Copyright (C) 2008, 2011, 2012, 2013, 2016 Benjamin Scott   <benscott@nwlink.com>
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

#include <QString>

#include "logvol.h"
#include "volgroup.h"

// MountEntry provides a thin wrapper for 'mntent' structs by providing
// QStrings rather than const char arrays and a 'mount position' property

MountEntry::MountEntry(MountEntry *const copy, QObject *parent) : QObject(parent)
{
    m_device_name     = copy->getDeviceName();
    m_mount_point     = copy->getMountPoint();
    m_filesystem_type = copy->getFilesystemType();
    m_mount_options   = copy->getMountOptions();

    m_major = copy->getMajorNumber();
    m_minor = copy->getMinorNumber();

    m_mount_position = copy->getMountPosition();
}

MountEntry::MountEntry(mntent *const entry, QObject *parent) : QObject(parent)
{
    m_device_name     = QString(entry->mnt_fsname).trimmed();
    m_mount_point     = QString(entry->mnt_dir).trimmed();
    m_filesystem_type = QString(entry->mnt_type).trimmed();
    m_mount_options   = QString(entry->mnt_opts).trimmed();

    m_major = -1;
    m_minor = -1;

    m_mount_position = 0;
}

MountEntry::MountEntry(const QString mountinfo, const int major, const int minor, QObject *parent) : QObject(parent)
{
    m_mount_point     = mountinfo.section(' ', 4, 4).replace("\\040", " ");
    m_mount_options   = mountinfo.section(' ', 5, 5);
    m_filesystem_type = mountinfo.section(' ', 6, 6);
    m_device_name     = mountinfo.section(' ', 8, 8);
    m_super_options   = mountinfo.section(' ', 9, 9);

    m_major = major;
    m_minor = minor;
    m_mount_position = 0;
}

MountEntry::~MountEntry()
{
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
    QStringList options;

    options.append(m_mount_options.split(','));
    options.append(m_super_options.split(','));
    options.removeDuplicates();

    return options.join(",");
}

int MountEntry::getMountPosition()
{
    return m_mount_position;
}

int MountEntry::getMajorNumber()
{
    return m_major;
}

int MountEntry::getMinorNumber()
{
    return m_minor;
}

void MountEntry::setMountPosition(const int pos)
{
    m_mount_position = pos;
}
