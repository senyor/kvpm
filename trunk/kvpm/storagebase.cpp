/*
 *
 * 
 * Copyright (C) 2013  Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include <sys/types.h>
#include <kde_file.h>

#include <QByteArray>
#include <QDebug>

#include "physvol.h"
#include "storagebase.h"

StorageBase::StorageBase(PedPartition *const part, const QList<PhysVol *> &pvList)
{
    m_sector_size = part->disk->dev->sector_size;

    char *const ped_path = ped_partition_get_path(part);
    m_name = QString(ped_path).trimmed();
    free(ped_path);

    m_is_writable = !part->disk->dev->read_only;
    commonConstruction(pvList);

    if (m_is_pv)
        m_is_busy = m_pv->isActive();
    else
        m_is_busy = ped_partition_is_busy(part);
}

StorageBase::StorageBase(PedDevice *const device, const QList<PhysVol *> &pvList)
{
    m_sector_size = device->sector_size;
    m_name = QString(device->path).trimmed();
    m_is_writable = !device->read_only;
    commonConstruction(pvList);
    m_is_busy = ped_device_is_busy(device);
}


void StorageBase::commonConstruction(const QList<PhysVol *> &pvList)
{
    m_is_pv = false;
    m_pv = NULL;

    for (auto *pv : pvList) {
        if (m_name == pv->getName()) {
            m_pv = pv;
            m_is_pv = true;
        }
    }

    KDE_struct_stat buf;
    QByteArray path = m_name.toLocal8Bit();

    if (KDE_stat(path.data(), &buf) == 0) {
        m_major =  major(buf.st_rdev);
        m_minor =  minor(buf.st_rdev);
    } else {
        m_major = -1;
        m_minor = -1;
    }
}
