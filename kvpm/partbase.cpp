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


#include "partbase.h"

#include "dualselectorbox.h"
#include "fsreduce.h"
#include "pedexceptions.h"
#include "partitiongraphic.h"
#include "physvol.h"
#include "processprogress.h"
#include "progressbox.h"
#include "storagepartition.h"
#include "volgroup.h"

#include <math.h>

#include <KLineEdit>
#include <KApplication>
#include <KButtonGroup>
#include <KConfigSkeleton>
#include <KGlobal>
#include <KLocale>
#include <KMessageBox>
#include <KPushButton>

#include <QDebug>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>


// NOTE: the cylinder_alignment flag is set to 0 for all devices in StorageDevice
// Don't set it here because freespace "partitions" rearrange themselves for some
// reason if it gets set after the partitions are probed.

PartitionDialogBase::PartitionDialogBase(StoragePartition *const partition, QWidget *parent)
    : KDialog(parent),
      m_old_storage_part(partition)
{
    m_is_new = (partition->isFreespace() || partition->isEmptyExtended());

    if (m_is_new) {
        const unsigned ped_type = m_old_storage_part->getPedType();
        const bool logical_freespace = (ped_type & PED_PARTITION_FREESPACE) && (ped_type & PED_PARTITION_LOGICAL);
        const int count = ped_disk_get_primary_partition_count(m_old_storage_part->getPedPartition()->disk);
        const int max_count = ped_disk_get_max_primary_partition_count(m_old_storage_part->getPedPartition()->disk);
        
        if (count >= max_count  && (!(logical_freespace || m_old_storage_part->isExtended()))) {
            KMessageBox::error(0, i18n("This disk already has %1 primary partitions, the maximum", count));
            m_bailout = true;
        } else if (m_old_storage_part->isExtended() && (!m_old_storage_part->isEmptyExtended())) {
            KMessageBox::error(0, i18n("This should not happen. Try selecting the freespace and not the partiton itself"));
            m_bailout = true;
        } else {
            m_bailout = false;
        }
    } else {
        m_bailout = false;
    }
    
    if (!m_bailout) {
        m_existing_part = m_old_storage_part->getPedPartition();
        m_current_size  = m_existing_part->geom.length;
        m_current_start = m_existing_part->geom.start;
        
        PedDisk *const disk = m_existing_part->disk;
        m_sector_size = disk->dev->sector_size;
        const PedSector ONE_MIB   = 0x100000 / getSectorSize();
        
        if (m_is_new) {
            m_min_shrink_size = 1;
 
            if (!setMaxFreespace(m_max_start, m_max_end)) {
                KMessageBox::error(0, i18n("No free space found"));
                m_bailout = true;
            }

            if (getMaxSize() < (3 * ONE_MIB)) {
                KMessageBox::error(0, i18n("Not enough free space for a new partition"));
                m_bailout = true;
            }
        } else {
            const PedSector current_end = (getCurrentSize() + getCurrentStart()) - 1;

            m_min_shrink_size = setMinSize();
            setMaxPart(m_max_start, m_max_end);

            if (((getCurrentSize() - getMinSize()) < ONE_MIB) && 
                ((getMaxEnd() - current_end) < ONE_MIB)       &&
                ((getCurrentStart() - getMaxStart()) < ONE_MIB)) {

                KMessageBox::error(0, i18n("Not enough free space to move or extend this partition and it can not be shrunk"));
                m_bailout = true;
            }

            m_path = QString(ped_partition_get_path(m_existing_part));
        }
    }        

    if (!m_bailout) {
        buildDialog();
    }
}

void PartitionDialogBase::buildDialog()
{
    const PedSector max_size = 1 + (m_max_end - m_max_start);
    const PedSector existing_offset = m_existing_part->geom.start - m_max_start;
    const PedSector max_offset = max_size - m_min_shrink_size;
    
    m_display_graphic = new PartitionGraphic(m_sector_size * max_size, m_is_new);
    
    setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Reset);
    setWindowTitle(i18n("Move or resize a partition"));
    
    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    m_layout = new QVBoxLayout();
    
    QLabel *label = new QLabel();
    
    if (m_is_new)
        label->setText(i18n("<b>Create A New Partition</b>"));
    else
        label->setText(i18n("<b>Resize Or Move A Partition</b>"));
    
    label->setAlignment(Qt::AlignCenter);
    m_layout->addSpacing(5);
    m_layout->addWidget(label);
    m_layout->addSpacing(5);
    label = new QLabel(i18n("<b>Device: %1</b>", m_old_storage_part->getName()));
    label->setAlignment(Qt::AlignHCenter);
    m_layout->addWidget(label);
    m_layout->addSpacing(5);
    
    m_layout->addWidget(m_display_graphic);
    
    if (m_old_storage_part->isPhysicalVolume()) {
        if (m_old_storage_part->getPhysicalVolume()->isActive()) {
            m_dual_selector = new DualSelectorBox(m_sector_size, max_size,
                                                  m_min_shrink_size, max_size - existing_offset, m_existing_part->geom.length,
                                                  existing_offset, existing_offset, existing_offset);
        } else {
            m_dual_selector = new DualSelectorBox(m_sector_size, max_size,
                                                  m_min_shrink_size, max_size, m_existing_part->geom.length,
                                                  0, max_offset, existing_offset);
        }
    } else {
        m_dual_selector = new DualSelectorBox(m_sector_size, max_size,
                                              m_min_shrink_size, max_size, m_existing_part->geom.length,
                                              0, max_offset, existing_offset);
    }
    
    m_layout->addWidget(m_dual_selector);
    
    dialog_body->setLayout(m_layout);
    
    m_dual_selector->resetSelectors();
    
    connect(m_dual_selector, SIGNAL(changed()),
            this, SIGNAL(changed()));
    
    connect(this, SIGNAL(resetClicked()),
            m_dual_selector, SLOT(resetSelectors()));
    
    setDefaultButton(KDialog::Ok);
}

void PartitionDialogBase::updateGraphicAndLabels()
{
    const long long current = m_dual_selector->getCurrentSize() * m_sector_size;
    const long long offset = m_dual_selector->getCurrentOffset() * m_sector_size;

    if (m_is_new) {
        m_display_graphic->update(current, offset);
    } else {
        const long long change = m_sector_size * (m_dual_selector->getCurrentSize() - m_existing_part->geom.length);
        const long long move = m_sector_size * (m_dual_selector->getCurrentOffset() - (m_existing_part->geom.start - m_max_start));

        m_display_graphic->update(current, offset, move, change);
    }
}

bool PartitionDialogBase::setMaxFreespace(PedSector &start, PedSector &end)
{
    PedPartition *const real_free = getPedPartition();
    const PedSector sector   = real_free->geom.start;
    PedDisk *const disk      = ped_disk_duplicate(real_free->disk);
    PedDevice  *const device = disk->dev;
    PedPartition *free_space = ped_disk_get_partition_by_sector(disk, sector);

    // If this is an empty extended partition and not freespace inside
    // one then look for the freespace.
    // Also, ped_disk_get_partition_by_sector(disk, sector) may return a metadata
    // area instead of free space.

    if (m_old_storage_part->isExtended()) {
        while (!((free_space->type & PED_PARTITION_FREESPACE) && (free_space->type & PED_PARTITION_LOGICAL))) {
            free_space = ped_disk_next_partition(disk, free_space);

            if (!free_space) {
                qDebug() << "Extended partition with no freespace found!";
                return false;  // bail out
            }
        }
    }

    start = free_space->geom.start;
    end = free_space->geom.length + start - 1;

    PedPartition *part;
    if (free_space->type & PED_PARTITION_LOGICAL)
        part = ped_partition_new(disk, PED_PARTITION_LOGICAL, NULL, start, end);
    else
        part = ped_partition_new(disk, PED_PARTITION_NORMAL, NULL, start, end);

    start = part->geom.start;
    PedSector length = part->geom.length;

    const PedSector ONE_MIB = 0x100000 / m_sector_size;   // sectors per megabyte
    PedAlignment *const start_align  = ped_alignment_new(0, ONE_MIB);
    PedAlignment *const end_align    = ped_alignment_new(-1, ONE_MIB);
    PedGeometry  *const start_range  = ped_geometry_new(device, start, length);
    PedGeometry  *const end_range    = ped_geometry_new(device, start, length);

    PedConstraint *constraint = ped_constraint_new(start_align, end_align,
                                                   start_range, end_range,
                                                   1, length);

    ped_disk_add_partition(disk, part, constraint);

    PedGeometry *max_geometry = ped_disk_get_max_partition_geometry(disk, part, constraint);
    start = max_geometry->start;
    end = max_geometry->length + max_geometry->start - 1;

    ped_disk_remove_partition(disk, part);
    ped_partition_destroy(part);
    ped_disk_destroy(disk);
    ped_constraint_destroy(constraint);
    ped_geometry_destroy(max_geometry);

    return true;
}

void PartitionDialogBase::setMaxPart(PedSector &start, PedSector &end)
{
    PedPartition *const real_part = getPedPartition();
    const PedSector sector = real_part->geom.start;
    PedDisk      *const disk = ped_disk_duplicate(real_part->disk);
    PedDevice  *const device = disk->dev;
    PedPartition *const part = ped_disk_get_partition_by_sector(disk, sector);

    const PedSector ONE_MIB = 0x100000 / m_sector_size;

    PedSector const old_start = part->geom.start;
    PedSector const old_end   = part->geom.length + old_start - 1;

    PedConstraint *constraint = ped_constraint_any(device);
    PedGeometry *max_geometry = ped_disk_get_max_partition_geometry(disk, part, constraint);
    start = max_geometry->start;
    end = max_geometry->length + max_geometry->start - 1;
    ped_constraint_destroy(constraint);

    PedAlignment *const start_align  = ped_alignment_new(0, ONE_MIB);
    PedAlignment *const end_align    = ped_alignment_new(-1, ONE_MIB);
    PedGeometry  *const start_range  = ped_geometry_new(device, start, max_geometry->length);
    PedGeometry  *const end_range    = ped_geometry_new(device, start, max_geometry->length);

    constraint = ped_constraint_new(start_align, end_align,
                                    start_range, end_range,
                                    1, max_geometry->length);

    ped_geometry_destroy(max_geometry);
    max_geometry = ped_disk_get_max_partition_geometry(disk, part, constraint);
    start = max_geometry->start;
    end = max_geometry->length + max_geometry->start - 1;

    // Don't return a size smaller than partition already is
    if (start > old_start)
        start = old_start;
    if (end < old_end)
        end = old_end;

    ped_disk_destroy(disk);
    ped_constraint_destroy(constraint);
    ped_geometry_destroy(max_geometry);
}

PedSector PartitionDialogBase::getNewOffset()
{
    return m_dual_selector->getCurrentOffset();
}

PedSector PartitionDialogBase::getNewSize()
{
    return m_dual_selector->getCurrentSize();
}

PedSector PartitionDialogBase::getSectorSize()
{
    return m_sector_size;
}

PedSector PartitionDialogBase::getMaxSize()
{
    return 1 + (m_max_end - m_max_start);
}

PedSector PartitionDialogBase::getMinSize()
{
    return m_min_shrink_size;
}

PedSector PartitionDialogBase::getMaxStart()
{
    return m_max_start;
}

PedSector PartitionDialogBase::getMaxEnd()
{
    return m_max_end;
}

PedSector PartitionDialogBase::getCurrentStart()
{
    return m_current_start;
}

PedSector PartitionDialogBase::getCurrentSize()
{
    return m_current_size;
}

PedSector PartitionDialogBase::setMinSize()
{
    PedSector min;

    const QString fs = m_old_storage_part->getFilesystem();

    if (m_old_storage_part->isPhysicalVolume()) {
        
        PhysVol *const pv = m_old_storage_part->getPhysicalVolume();
        const long mda_count = pv->getMdaCount();
        const long long extent_size = pv->getVg()->getExtentSize();
        const long long mda_extents = (pv->getMdaSize() / extent_size) + 1;
        
        min = 1 + (mda_extents * mda_count) + pv->getLastUsedExtent();
        min *= extent_size;
        min /= m_sector_size;
    } else if (m_is_new) {
        min = 1;
    } else {
        if (fs_can_reduce(fs))
            min = get_min_fs_size(ped_partition_get_path(m_existing_part), fs) / m_sector_size;
        else
            min = m_existing_part->geom.length;
    }
    
    const PedSector TWO_MIB = 0x200000 / m_sector_size;
    
    if (min == 0)                            // 0 means we can't shrink it
        min = m_existing_part->geom.length;
    else if (min <= TWO_MIB) {               // Don't allow shrinking or new partitions below 2 MiB
        if (m_existing_part->geom.length > TWO_MIB)
            min  = TWO_MIB;
        else
            min  = m_existing_part->geom.length;
    }
    
    return min;
}

PedPartition *PartitionDialogBase::getPedPartition()
{
    return m_existing_part;
}

bool PartitionDialogBase::isValid()
{
    return m_dual_selector->isValid();
}

QString PartitionDialogBase::getPath()
{
    return m_path;
}

void PartitionDialogBase::insertWidget(QWidget *widget)
{
    m_layout->insertWidget(6, widget);   
}

/* The following function waits for udev to acknowledge the partion changes before exiting
   It also sets what getCurrentSize() and getCurrentStart() return to the new current values  */

bool PartitionDialogBase::pedCommitAndWait(PedDisk *disk)
{
    QStringList args;

    qApp->setOverrideCursor(Qt::WaitCursor);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    m_current_size  = m_existing_part->geom.length;
    m_current_start = m_existing_part->geom.start;

    if (!ped_disk_commit(disk)) {
        qApp->restoreOverrideCursor();
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        return false;
    } else {
        args << "udevadm" << "settle";
        ProcessProgress wait_settle(args);
        qApp->restoreOverrideCursor();
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        return true;
    }
}

bool PartitionDialogBase::hasInitialErrors()
{
    return m_bailout;
}
