/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef STORAGEPARTITION_H
#define STORAGEPARTITION_H

#include <parted/parted.h>

#include <QObject>
#include <QStringList>

#include "storagebase.h"

class PhysVol;
class MountEntry;
class MountTables;

class StoragePartition : public StorageBase
{
    QList<MountEntry *> m_mount_entries;
    QString m_fstab_mount_point;
    QStringList m_mount_points;
    PedPartition *m_ped_partition;
    QString m_name;
    QString m_partition_type;
    unsigned int m_ped_type;
    QString m_fs_type;
    QString m_fs_uuid;
    QString m_fs_label;
    QStringList m_flags;
    long long m_partition_size; // Partition size and first sector for *freespace* are aligned
    PedSector m_first_sector;   // within the space to 1 MiB and may be different than libparted reports.
    PedSector m_last_sector;    // Last sector is not aligned and will be the same as libarted
    PedSector m_true_first_sector;   // unaligned first sector
    long long m_fs_size;
    long long m_fs_used;
 
  
    bool m_is_mounted;
    bool m_is_extended;
    bool m_is_empty;   // empty extended partition
 
    bool m_is_mountable;
    bool m_is_normal;
    bool m_is_logical;
    bool m_is_freespace;

    PedSector getAlignedStart();
    PedSector getFreespaceEnd();

public:
    StoragePartition(PedPartition *const part, const int freespaceCount,
                     const QList<PhysVol *> pvList, MountTables *const tables);

    ~StoragePartition();

    QString getName() const { return m_name; }
    PedPartition *getPedPartition() const { return m_ped_partition; }
    QString getFilesystem() const { return m_fs_type; }
    QString getFilesystemUuid() const { return m_fs_uuid; }
    QString getFilesystemLabel() const { return m_fs_label; }
    QString getType() const { return m_partition_type; }
    unsigned int getPedType() const { return m_ped_type; }
    QString getFstabMountPoint() const { return m_fstab_mount_point; }
    QStringList getMountPoints() const { return m_mount_points; }
    QList<MountEntry *> getMountEntries() const ;  // These need to be deleted by the calling function!
    QStringList getFlags() const { return m_flags; }
    long long getSize() const { return m_partition_size; }
    PedSector getFirstSector() const { return m_first_sector; }
    PedSector getTrueFirstSector() const { return m_true_first_sector; }
    PedSector getLastSector() const { return m_last_sector; }
    long long getFilesystemSize() const { return m_fs_size; }
    long long getFilesystemUsed() const { return m_fs_used; }
    long long getFilesystemRemaining() const { return m_fs_size - m_fs_used; }
    int getFilesystemPercentUsed() const ;
    bool isMounted() const { return m_is_mounted; }
    bool isEmptyExtended() const ;
    bool isExtended() const { return m_is_extended; }
    bool isMountable() const { return m_is_mountable; }
    bool isNormal() const { return m_is_normal; }
    bool isLogical() const { return m_is_logical; }
    bool isFreespace() const { return m_is_freespace; }
};

#endif
