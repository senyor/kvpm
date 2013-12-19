/*
 *
 *
 * Copyright (C) 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#ifndef STORAGEBASE_H
#define STORAGEBASE_H


#include <parted/parted.h>

#include <QList>
#include <QStringList>


class PhysVol;

class StorageBase
{
    long long m_sector_size; // Size in bytes
    QString   m_name;
    bool      m_is_writable;
    bool      m_is_busy;
    bool      m_is_pv;
    bool      m_is_dmraid;
    bool      m_is_dmraid_block;
    PhysVol  *m_pv;
    int m_major;            // block dev numbers
    int m_minor;

    void commonConstruction(const QList<PhysVol *> &pvList);

public:
    StorageBase(PedDevice *const device, const QList<PhysVol *> &pvList, const QStringList dmblock, const QStringList dmraid);
    StorageBase(PedPartition *const part, const QList<PhysVol *> &pvList);
    virtual ~StorageBase() {}

    virtual long long getSize() const = 0;
    virtual QString getName() const { return m_name; }              // the full device path
    PhysVol *getPhysicalVolume() const { return m_pv; }
    long long getSectorSize() const { return m_sector_size; }   // Size in bytes
    int getMajorNumber() const { return m_major; }
    int getMinorNumber() const { return m_minor; }
    bool isWritable() const { return m_is_writable; }
    bool isBusy() const { return m_is_busy; }
    bool isPhysicalVolume() const { return m_is_pv; }
    bool isDmRaid() const { return m_is_dmraid; }             // dmraid device ie: /dev/mapper/foo
    bool isDmRaidBlock() const { return m_is_dmraid_block; }  // real block device under dmraid 
};

#endif
