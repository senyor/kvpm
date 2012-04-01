/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "executablefinder.h"

#include <kde_file.h>
#include <KConfigSkeleton>

#include <QtGui>


QMap<QString, QString> ExecutableFinder::m_path_map = QMap<QString, QString>(); // Static initialization


/* The purpoise of this class is to map the name of a program
   with the full path of the executable */

ExecutableFinder::ExecutableFinder(QObject *parent) : QObject(parent)
{
    m_keys << "dumpe2fs"
           << "fsck"
           << "lvchange"
           << "lvconvert"
           << "lvcreate"
           << "lvextend"
           << "lvreduce"
           << "lvremove"
           << "lvrename"
           << "lvs"
           << "mkfs"
           << "mkswap"
           << "ntfsresize"
           << "pvchange"
           << "pvcreate"
           << "pvmove"
           << "pvremove"
           << "pvresize"
           << "pvs"
           << "resize2fs"
           << "resize_reiserfs"
           << "udevadm"
           << "vgchange"
           << "vgcreate"
           << "vgexport"
           << "vgextend"
           << "vgimport"
           << "vgmerge"
           << "vgreduce"
           << "vgremove"
           << "vgrename"
           << "vgs"
           << "vgsplit"
           << "xfs_growfs";

    m_default_search_paths << "/sbin/"
                           << "/usr/sbin/"
                           << "/bin/"
                           << "/usr/bin/"
                           << "/usr/local/bin/"
                           << "/usr/local/sbin/";

    reload();

}

QString ExecutableFinder::getPath(const QString name)
{
    QString path = m_path_map.value(name);

    if (path.isEmpty())
        qDebug() << "Excutable Finder: error " << name  << " does not map to any path";

    return path;
}

void ExecutableFinder::reload()
{
    KConfigSkeleton skeleton;
    QStringList search;

    skeleton.setCurrentGroup("SystemPaths");
    skeleton.addItemStringList("SearchPath", search, m_default_search_paths);

    reload(search);
}

void ExecutableFinder::reload(QStringList search)
{
    KDE_struct_stat buf;
    const int key_length = m_keys.size();

    m_path_map.clear();

    for (int y = 0; y < key_length; y++) {
        for (int x = 0; x < search.size(); x++) {

            QByteArray path_qba = QString(search[x] + m_keys[y]).toLocal8Bit();
            const char *path = path_qba.data();

            if (KDE_lstat(path, &buf) == 0) {
                m_path_map.insert(m_keys[y], search[x] + m_keys[y]);
                break;
            }
        }
    }

    m_not_found.clear();
    for (int x = 0; x < key_length; x++) {
        if ((m_path_map.value(m_keys[x])).isEmpty())
            m_not_found.append(m_keys[x]);
    }
}

QStringList ExecutableFinder::getAllPaths()
{
    return m_path_map.values();
}


QStringList ExecutableFinder::getAllNames()
{
    return   m_path_map.keys();
}

QStringList ExecutableFinder::getNotFound()
{
    return m_not_found;
}
