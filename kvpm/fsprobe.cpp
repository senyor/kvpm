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


#include "fsprobe.h"

#include <blkid/blkid.h>

#include <QtGui>

#include "logvol.h"
#include "masterlist.h"
#include "storagedevice.h"
#include "storagepartition.h"
#include "volgroup.h"

extern MasterList *g_master_list;

#define BLKID_EMPTY_CACHE       "/dev/null"

QString fsprobe_getfstype2(QString devicePath)
{
    static blkid_cache blkid2;
    const QByteArray path = devicePath.toLocal8Bit();
    QString fs_type;
    blkid2 = NULL;
    
    if( blkid_get_cache(&blkid2, BLKID_EMPTY_CACHE) < 0){
	qDebug() << "blkid2 cache could not be allocated?";
	return QString();
    }
    else{
	fs_type = QString( blkid_get_tag_value(blkid2, "TYPE", path.data() ) );
	blkid_put_cache(blkid2);
	return fs_type;
    }
}

QString fsprobe_getfsuuid(QString devicePath)
{
    static blkid_cache blkid2;
    const QByteArray path = devicePath.toLocal8Bit();
    QString fs_uuid;
    blkid2 = NULL;
    
    if( blkid_get_cache(&blkid2, BLKID_EMPTY_CACHE) < 0){
	qDebug() << "blkid2 cache could not be allocated?";
	return QString();
    }
    else{
	fs_uuid = QString( blkid_get_tag_value(blkid2, "UUID", path.data() ) );
	blkid_put_cache(blkid2);
	return fs_uuid;
    }
}

QString fsprobe_getfslabel(QString devicePath)
{
    static blkid_cache blkid2;
    const QByteArray path = devicePath.toLocal8Bit();
    QString fs_label;
    blkid2 = NULL;
    
    if( blkid_get_cache(&blkid2, BLKID_EMPTY_CACHE) < 0){
	qDebug() << "blkid2 cache could not be allocated?";
	return QString();
    }
    else{
        fs_label = QString( blkid_get_tag_value(blkid2, "LABEL", path.data() ) );
	blkid_put_cache(blkid2);
	return fs_label;
    }
}

QString fsprobe_getfs_by_uuid(QString uuid)
{
    QList<VolGroup *> vgs = g_master_list->getVolGroups();
    QList<StorageDevice *> devs = g_master_list->getStorageDevices();
    QList<StoragePartition *> parts;
    QList<LogVol *> lvs;

    uuid = uuid.trimmed();
    if( uuid.startsWith("UUID=", Qt::CaseInsensitive) )
        uuid = uuid.remove(0, 5);

    for(int x = devs.size() - 1; x >= 0; x--){
        parts = devs[x]->getStoragePartitions();
        for(int y = parts.size() - 1; y >= 0; y--){
            if( parts[y]->getFilesystemUuid() == uuid )
                return parts[y]->getName();
        }
    }

    for(int x = vgs.size() - 1; x >= 0; x--){
        lvs = vgs[x]->getLogicalVolumes();
        for(int y = lvs.size() - 1; y >= 0; y--){
            if( lvs[y]->getFilesystemUuid() == uuid )
                return lvs[y]->getMapperPath();
        }
    }

    return QString();
}

QString fsprobe_getfs_by_label(QString label)
{
    QList<VolGroup *> vgs = g_master_list->getVolGroups();
    QList<StorageDevice *> devs = g_master_list->getStorageDevices();
    QList<StoragePartition *> parts;
    QList<LogVol *> lvs;

    label = label.trimmed();
    if( label.startsWith("LABEL=", Qt::CaseInsensitive) )
        label = label.remove(0, 6);

    for(int x = devs.size() - 1; x >= 0; x--){
        parts = devs[x]->getStoragePartitions();
        for(int y = parts.size() - 1; y >= 0; y--){
            if( parts[y]->getFilesystemLabel() == label )
                return parts[y]->getName();
        }
    }

    for(int x = vgs.size() - 1; x >= 0; x--){
        lvs = vgs[x]->getLogicalVolumes();
        for(int y = lvs.size() - 1; y >= 0; y--){
            if( lvs[y]->getFilesystemLabel() == label )
                return lvs[y]->getMapperPath();
        }
    }

    return QString();
}
