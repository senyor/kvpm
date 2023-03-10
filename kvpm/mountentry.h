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

#ifndef MOUNTENTRY_H
#define MOUNTENTRY_H

#include <QObject>

class QString;

class mntent;


class MountEntry : public QObject
{
    Q_OBJECT

    QString m_device_name,        // for example: "/dev/sda1"
            m_mount_point,
            m_filesystem_type,    // ext3, reiserfs, swap etcetera
            m_mount_options,      // options (per mount), such as "noatime," set when mounting a filesystem
            m_super_options;      // options (per superblock) set when mounting a filesystem

    int m_mount_position;         // More than on device may be mounted on a mount point.
    // This number is zero if nothing else is mounted on
    // this mount point. Otherwise numbers go in reverse
    // of mount order. 1 is the *last* one mounted, highest
    // number is the first one mounted.

    int m_major, m_minor;

public:
    explicit MountEntry(MountEntry *const copy, QObject *parent = 0);
    explicit MountEntry(mntent *const entry, QObject *parent = 0);
    ~MountEntry();
    MountEntry(QString const mountinfo, const int major, const int minor, QObject *parent = 0);
    QString getDeviceName();
    QString getMountPoint();
    QString getFilesystemType();
    QString getMountOptions();
    int getMountPosition();
    int getMajorNumber();
    int getMinorNumber();
    void setMountPosition(const int pos);
};

#endif
