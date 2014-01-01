/*
 *
 * 
 * Copyright (C) 2013, 2014  Benjamin Scott   <benscott@nwlink.com>
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



StorageBase::StorageBase(PedPartition *const part, const QList<PhysVol *> &pvList, const QStringList mdblock)
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

    m_is_dmraid = false;
    m_is_dmraid_block = false;
    m_is_mdraid = false;
    m_is_mdraid_block = false;

    for (auto dev : mdblock) {
        if (dev == m_name) {
            m_is_mdraid_block = true;
            break;
        }
    }
}

StorageBase::StorageBase(PedDevice *const device, const QList<PhysVol *> &pvList, 
                         const QStringList dmblock, const QStringList dmraid,
                         const QStringList mdblock, const QStringList mdraid)
{
    m_sector_size = device->sector_size;
    m_name = QString(device->path).trimmed();
    m_is_writable = !device->read_only;
    commonConstruction(pvList);
    m_is_busy = ped_device_is_busy(device);

    m_is_dmraid = false;
    m_is_dmraid_block = false;
    m_is_mdraid = false;
    m_is_mdraid_block = false;

    for (auto dev : dmblock) {
        if (dev == m_name) {
            m_is_dmraid_block = true;
            break;
        }
    }

    for (auto dev : dmraid) {
        if (dev == m_name) {
            m_is_dmraid = true;
            break;
        }
    }

    for (auto dev : mdblock) {
        if (dev == m_name) {
            m_is_mdraid_block = true;
            break;
        }
    }

    for (auto dev : mdraid) {
        if (dev == m_name) {
            m_is_mdraid = true;
            break;
        }
    }
}

void StorageBase::commonConstruction(const QList<PhysVol *> &pvList)
{
    m_is_pv = false;
    m_pv = NULL;

    for (auto *pv : pvList) {
        if (m_name == pv->getMapperName()) {
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
