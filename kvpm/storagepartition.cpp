/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "storagepartition.h"

#include <sys/types.h>
#include <kde_file.h>

#include <QDebug>

#include "fsdata.h"
#include "fsprobe.h"
#include "misc.h"
#include "mountentry.h"
#include "mounttables.h"
#include "physvol.h"



StoragePartition::StoragePartition(PedPartition *const part, const int freespaceCount, 
                                   const QList<PhysVol *> pvList, MountTables *const tables)
    : StorageBase(part, pvList), m_ped_partition(part)
{
    PedDevice *const ped_device = m_ped_partition->disk->dev;
    PedGeometry const geometry  = m_ped_partition->geom;
    long long const sector_size = ped_device->sector_size;
    m_true_first_sector = geometry.start;
    m_first_sector   = geometry.start;
    m_last_sector    = geometry.end;
    m_partition_size = geometry.length * sector_size; // in bytes
    m_ped_type       = m_ped_partition->type;

    const bool is_pv = isPhysicalVolume();
 
    m_is_normal   = false;
    m_is_extended = false;
    m_is_logical  = false;
    m_is_freespace = false;

    if (m_ped_type & PED_PARTITION_FREESPACE) {
        if (m_ped_type & PED_PARTITION_LOGICAL) {
            m_last_sector = getFreespaceEnd();
        }

        m_first_sector = getAlignedStart();
        m_is_freespace = true;
        m_partition_size = (1 + m_last_sector - m_first_sector) * sector_size;
    }

    char *const ped_path = ped_partition_get_path(part);
    m_name = QString(ped_path);
    free(ped_path);

    if (m_ped_type == PED_PARTITION_NORMAL) {
        m_partition_type = "normal";
        m_is_normal = true;
    } else if (m_ped_type & PED_PARTITION_EXTENDED) {
        m_partition_type = "extended";
        m_is_extended = true;
    } else if ((m_ped_type & PED_PARTITION_LOGICAL) && !(m_ped_type & PED_PARTITION_FREESPACE)) {
        m_partition_type = "logical";
        m_is_logical = true;

        // NOTE: the device paths in PED for freespace are always numered "1" (like /dev/sda-1).
        // We change the "1" to an incremented number for each expanse of freespace found.

    } else if ((m_ped_type & PED_PARTITION_LOGICAL) && (m_ped_type & PED_PARTITION_FREESPACE)) {
        m_partition_type = "freespace (logical)";
        m_name.chop(1);
        m_name.append(QString("%1").arg(freespaceCount));
    } else {
        m_partition_type = "freespace";
        m_name.chop(1);
        m_name.append(QString("%1").arg(freespaceCount));
    }

    m_name = m_name.trimmed();
    m_partition_type = m_partition_type.trimmed();

    // Iterate though all the possible flags and check each one
    PedPartitionFlag ped_flag = PED_PARTITION_BOOT;

    if (!m_partition_type.contains("freespace", Qt::CaseInsensitive)) {
        while (ped_flag != 0) {
            if (ped_partition_get_flag(m_ped_partition, ped_flag))
                m_flags << ped_partition_flag_get_name(ped_flag);

            ped_flag = ped_partition_flag_next(ped_flag);
        }
        if (!m_flags.size())
            m_flags << "";
    }

    if (m_partition_type == "extended") {
        m_is_mountable = false;
        m_fs_type = "";
    } else {
        m_fs_type  = fsprobe_getfstype2(m_name).trimmed();
        m_fs_uuid  = fsprobe_getfsuuid(m_name).trimmed();
        m_fs_label = fsprobe_getfslabel(m_name).trimmed();

        if (m_fs_type == "swap" || is_pv || m_partition_type.contains("freespace", Qt::CaseInsensitive))
            m_is_mountable = false;
        else
            m_is_mountable = true;
    }

    // This isn't needed until we start reusing StoragePartition

    for (int x = m_mount_entries.size() - 1; x >= 0; --x)
        delete m_mount_entries.takeAt(x);

    m_mount_entries = tables->getMtabEntries(getMajorNumber(), getMinorNumber());
    m_fstab_mount_point = tables->getFstabMountPoint(this);
    m_is_mounted = !m_mount_entries.isEmpty();

    for (auto entry : m_mount_entries)
        m_mount_points.append(entry->getMountPoint());

    if (m_is_mounted) {
        FSData fs_data = get_fs_data(m_mount_entries[0]->getMountPoint());
        if (fs_data.size > 0) {
            m_fs_size = fs_data.size * fs_data.block_size;
            m_fs_used = fs_data.used * fs_data.block_size;
        } else {
            m_fs_size = -1;
            m_fs_used = -1;
        }
    } else {
        m_fs_size = -1;
        m_fs_used = -1;
    }

    if (m_partition_type == "extended") {

        PedPartition *temp_part = NULL;
        PedDisk *const temp_disk = ped_disk_new(ped_device);

        m_is_empty = true;
        if (temp_disk) {
            while ((temp_part = ped_disk_next_partition(temp_disk, temp_part))) {
                if ((temp_part->type  == PED_PARTITION_LOGICAL)) {
                    m_is_empty = false;
                    break;
                }
            }
            ped_disk_destroy(temp_disk);
        }
    } else {
        m_is_empty = false;
    }
}

StoragePartition::~StoragePartition()
{
    for (int x = 0; x < m_mount_entries.size(); x++)
        delete m_mount_entries[x];
}

int StoragePartition::getFilesystemPercentUsed() const
{
    int percent;

    if (m_fs_used == 0)
        return 0;
    else if (m_fs_used == m_fs_size)
        return 100;
    else if (m_fs_size == 0)
        return 0;
    else
        percent = qRound(100 - (((m_fs_size - m_fs_used) * 100.0) / m_fs_size));

    return percent;
}

/* This function returns true if the partition is extended
   and has no logical partitions */

bool StoragePartition::isEmptyExtended() const
{
    return m_is_empty;
}

QList<MountEntry *> StoragePartition::getMountEntries() const
{
    QList<MountEntry *> copy;
    QListIterator<MountEntry *> itr(m_mount_entries);

    while (itr.hasNext())
        copy.append(new MountEntry(itr.next()));

    return copy;
}

PedSector StoragePartition::getAlignedStart()
{
    PedPartition *const free_space = m_ped_partition;
    PedDevice *const device = free_space->disk->dev;
    const PedSector ONE_MIB = 0x100000 / device->sector_size;   // sectors per megabyte

    PedSector start = free_space->geom.start;
    PedSector const end = free_space->geom.length + start - 1;

    if ((end - start) < (ONE_MIB * 2))   // ignore partitions less than 2 MiB, alignment may fail.
        return start;

    PedAlignment *const start_align = ped_alignment_new(0, ONE_MIB);
    PedAlignment *const end_align   = ped_alignment_new(0, 1);
    PedGeometry  *const start_range = ped_geometry_new(device, start, free_space->geom.length);
    PedGeometry  *const end_range   = ped_geometry_new(device, start, free_space->geom.length);

    PedConstraint *constraint = ped_constraint_new(start_align, end_align,
                                start_range, end_range,
                                1, free_space->geom.length);

    PedGeometry *const aligned_geometry = ped_constraint_solve_max(constraint);

    start = aligned_geometry->start;

    ped_alignment_destroy(start_align);
    ped_alignment_destroy(end_align);
    ped_constraint_destroy(constraint);
    ped_geometry_destroy(start_range);
    ped_geometry_destroy(end_range);
    ped_geometry_destroy(aligned_geometry);

    return start;
}


/* The following function works *only* on logical freespace.
   It finds the largest partition that will fit in the freespace
   by adding any space in a following metadata area, if one exists */

PedSector StoragePartition::getFreespaceEnd()
{
    PedPartition *const free_space = m_ped_partition;
    PedDisk   *const disk = free_space->disk;
    PedSector end = free_space->geom.length + free_space->geom.start - 1;
    PedPartition *next_part = ped_disk_next_partition(disk, free_space);

    if (next_part) {
        if (next_part->type & PED_PARTITION_METADATA) {
            end = next_part->geom.length + next_part->geom.start - 1;
        }
    }

    return end;
}
