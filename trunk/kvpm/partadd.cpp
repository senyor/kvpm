/*
 *
 *
 * Copyright (C) 2009, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "partadd.h"

#include "dualselectorbox.h"
#include "partitiongraphic.h"
#include "pedexceptions.h"
#include "sizeselectorbox.h"
#include "storagepartition.h"

#include <KComboBox>
#include <KConfigSkeleton>
#include <KGlobal>
#include <KLocale>
#include <KLineEdit>
#include <KMessageBox>

#include <QCheckBox>
#include <QDebug>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>


/*
   The "partition" we get here is usually a ped pointer to a the freespace
   between partitions. It can also be an *empty* extended partition however.
*/

PartitionAddDialog::PartitionAddDialog(StoragePartition *const partition, QWidget *parent)
    : KDialog(parent),
      m_partition(partition)
{
    KConfigSkeleton skeleton;
    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", m_use_si_units, false);

    if (!(m_bailout = hasInitialErrors())) {

        getMaximumPartition(m_max_part_start, m_max_part_end, m_sector_size);
        m_max_part_size = m_max_part_end - m_max_part_start + 1;

        if (m_max_part_size >= (0x200000 / m_sector_size)) {   // sectors per megabyte

            setWindowTitle(i18n("Create A New Partition"));

            QWidget *const dialog_body = new QWidget(this);
            setMainWidget(dialog_body);
            QVBoxLayout *const layout = new QVBoxLayout();
            dialog_body->setLayout(layout);

            QLabel *const label = new QLabel(i18n("<b>Create A New Partition</b>"));
            label->setAlignment(Qt::AlignCenter);
            layout->addSpacing(5);
            layout->addWidget(label);
            layout->addSpacing(5);

            layout->addWidget(buildInfoGroup());
            m_dual_selector = new DualSelectorBox(m_sector_size, m_max_part_size);
            validateChange();

            layout->addWidget(m_dual_selector);

            connect(m_dual_selector, SIGNAL(changed()),
                    this, SLOT(validateChange()));

            connect(this, SIGNAL(okClicked()),
                    this, SLOT(commitPartition()));
        } else {
            m_bailout = true;
            KMessageBox::error(0, i18n("Not enough usable space for a new partition"));
        }
    }

    setDefaultButton(KDialog::Cancel);
}

void PartitionAddDialog::commitPartition()
{
    const PedSector ONE_MIB = 0x100000 / m_sector_size;   // sectors per megabyte
    PedDevice *const device = m_partition->getPedPartition()->disk->dev;
    PedDisk   *const disk   = m_partition->getPedPartition()->disk;
    PedSector first_sector  = m_max_part_start + m_dual_selector->getCurrentOffset();
    PedSector last_sector   = first_sector + m_dual_selector->getCurrentSize() - 1;
    PedPartitionType type;

    if (first_sector < m_max_part_start)
        first_sector = m_max_part_start;

    if (last_sector > m_max_part_end)
        last_sector = m_max_part_end;

    if ((1 + last_sector - first_sector) < (2 * ONE_MIB)) {
        KMessageBox::error(0, i18n("Partitions less than two MiB are not supported"));
        return;
    }

    PedAlignment *const start_align = ped_alignment_new(0, ONE_MIB);
    PedAlignment *const end_align   = ped_alignment_new(-1, ONE_MIB);

    PedGeometry  *const max_geom = ped_geometry_new(device, m_max_part_start, 1 + m_max_part_end - m_max_part_start);
    first_sector = ped_alignment_align_nearest(start_align, max_geom, first_sector);
    last_sector  = ped_alignment_align_nearest(end_align, max_geom, last_sector);
    ped_geometry_destroy(max_geom);

    const PedSector length = 1 + last_sector - first_sector;

    PedGeometry *const start_range = ped_geometry_new(device, first_sector, length);
    PedGeometry *const end_range   = ped_geometry_new(device, first_sector, length);

    PedConstraint *const constraint = ped_constraint_new(start_align, end_align,
                                      start_range, end_range,
                                      length - ONE_MIB, length);

    if (constraint != NULL) {
        if (m_type_combo->currentIndex() == 0)
            type = PED_PARTITION_NORMAL ;
        else if (m_type_combo->currentIndex() == 1)
            type = PED_PARTITION_EXTENDED ;
        else
            type = PED_PARTITION_LOGICAL ;

        PedDiskFlag cylinder_flag = ped_disk_flag_get_by_name("cylinder_alignment");
        if (ped_disk_is_flag_available(disk, cylinder_flag))
            ped_disk_set_flag(disk, cylinder_flag, 0);

        PedPartition *ped_new_partition = ped_partition_new(disk, type, 0, first_sector, last_sector);
        if (ped_new_partition != NULL) {
            if (ped_disk_add_partition(disk, ped_new_partition, constraint))
                ped_disk_commit(disk);
        }

        ped_constraint_destroy(constraint);
    }

    ped_geometry_destroy(start_range);
    ped_geometry_destroy(end_range);
}

long long PartitionAddDialog::convertSizeToSectors(int index, double size)
{
    long double partition_size = size;

    if (index == 0)
        partition_size *= (long double)0x100000;
    else if (index == 1)
        partition_size *= (long double)0x40000000;
    else {
        partition_size *= (long double)0x100000;
        partition_size *= (long double)0x100000;
    }

    partition_size /= m_sector_size;

    if (partition_size < 0)
        partition_size = 0;

    return qRound64(partition_size);
}

void PartitionAddDialog::validateChange()
{

    const PedSector ONE_MIB  = 0x100000 / m_sector_size;   // sectors per megabyte
    const long long offset = m_dual_selector->getCurrentOffset();
    const long long size   = m_dual_selector->getCurrentSize();

    if ((offset <= m_max_part_size - size) && (size <= m_max_part_size - offset) &&
            (size >= 2 * ONE_MIB) && (m_dual_selector->isValid())) {
        enableButtonOk(true);
    } else
        enableButtonOk(false);

    updatePartition();
}

void PartitionAddDialog::updatePartition()
{

    const long long offset = m_dual_selector->getCurrentOffset();
    const long long size = m_dual_selector->getCurrentSize();
    long long free = m_max_part_size - offset;
    if (free < 0)
        free = 0;

    KLocale *const locale = KGlobal::locale();
    if (m_use_si_units)
        locale->setBinaryUnitDialect(KLocale::MetricBinaryDialect);
    else
        locale->setBinaryUnitDialect(KLocale::IECBinaryDialect);

    QString preceding_bytes_string = locale->formatByteSize(offset * m_sector_size);
    m_preceding_label->setText(i18n("Preceding space: %1", preceding_bytes_string));

    PedSector following = m_max_part_size - (size + offset);
    if (following < 0)
        following = 0;

    PedSector following_space = following * m_sector_size;

    if (following_space < 32 * m_sector_size)
        following_space = 0;

    QString following_bytes_string = locale->formatByteSize(following_space);
    m_remaining_label->setText(i18n("Following space: %1", following_bytes_string));

    m_display_graphic->setPrecedingSectors(offset);
    m_display_graphic->setPartitionSectors(size);
    m_display_graphic->setFollowingSectors(following);
    m_display_graphic->repaint();
}

void PartitionAddDialog::getMaximumPartition(PedSector &start, PedSector &end, PedSector &sectorSize)
{
    PedPartition *free_space = m_partition->getPedPartition();
    PedDevice *const device = free_space->disk->dev;
    PedDisk   *const disk = ped_disk_new(device);
    sectorSize = device->sector_size;
    const PedSector ONE_MIB = 0x100000 / sectorSize;   // sectors per megabyte

    // If this is an empty extended partition and not freespace inside
    // one then look for the freespace.

    if (free_space->type & PED_PARTITION_EXTENDED) {
        do {
            free_space = ped_disk_next_partition(disk, free_space);
            if (!free_space)
                qDebug() << "Extended partition with no freespace found!";
        } while (!((free_space->type & PED_PARTITION_FREESPACE) && (free_space->type & PED_PARTITION_LOGICAL)));
    }

    start = free_space->geom.start;
    end = free_space->geom.length + start - 1;

    PedPartition *part;

    if (free_space->type & PED_PARTITION_LOGICAL)
        part = ped_partition_new(disk, PED_PARTITION_LOGICAL, NULL, start, end);
    else
        part = ped_partition_new(disk, PED_PARTITION_NORMAL, NULL, start, end);

    start = part->geom.start;
    end = part->geom.length + start - 1;

    PedConstraint *constraint = ped_constraint_any(device);
    ped_disk_add_partition(disk, part, constraint);

    start = part->geom.start;
    end = part->geom.length + start - 1;

    PedDiskFlag cylinder_flag = ped_disk_flag_get_by_name("cylinder_alignment");
    if (ped_disk_is_flag_available(disk, cylinder_flag))
        ped_disk_set_flag(disk, cylinder_flag, 0);

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

    ped_disk_remove_partition(disk, part);
    ped_partition_destroy(part);
    ped_constraint_destroy(constraint);
    ped_geometry_destroy(max_geometry);
    ped_disk_destroy(disk);
}

QGroupBox* PartitionAddDialog::buildInfoGroup()
{
    QGroupBox *const group = new QGroupBox(this);
    QVBoxLayout *const layout = new QVBoxLayout();
    layout->addSpacing(10);

    m_display_graphic = new PartitionGraphic();
    layout->addWidget(m_display_graphic, 0, Qt::AlignCenter);
    QLabel *const path = new QLabel(i18n("<b>Device: %1</b>", m_partition->getName()));
    path->setAlignment(Qt::AlignHCenter);
    layout->addWidget(path);
    layout->addSpacing(10);

    m_preceding_label = new QLabel();
    layout->addWidget(m_preceding_label);

    m_remaining_label = new QLabel();
    layout->addWidget(m_remaining_label);

    KLocale *const locale = KGlobal::locale();
    if (m_use_si_units)
        locale->setBinaryUnitDialect(KLocale::MetricBinaryDialect);
    else
        locale->setBinaryUnitDialect(KLocale::IECBinaryDialect);

    QString total_bytes = locale->formatByteSize(m_max_part_size * m_sector_size);
    QLabel *const excluded_label = new QLabel(i18n("Maximum size: %1",  total_bytes));
    layout->addWidget(excluded_label);
    layout->addSpacing(10);

    QLabel *const type_label = new QLabel(i18n("Select type: "));
    QHBoxLayout *const type_layout = new QHBoxLayout;
    m_type_combo = buildTypeCombo();
    type_label->setBuddy(m_type_combo);
    type_layout->addWidget(type_label);
    type_layout->addWidget(m_type_combo);
    type_layout->addStretch();
    layout->addLayout(type_layout);

    group->setLayout(layout);

    return group;
}

KComboBox* PartitionAddDialog::buildTypeCombo()
{
    KComboBox *const combo = new KComboBox;
    PedPartition *const free_space = m_partition->getPedPartition();

    bool logical_freespace;      // true if we are inside an extended partition
    bool extended_allowed;       // true if we can create an extended partition here

    /* check to see if partition table supports extended
       partitions and if it already has one */

    PedDisk *const disk  = m_partition->getPedPartition()->disk;

    if ((PED_DISK_TYPE_EXTENDED & disk->type->features) && (!ped_disk_extended_partition(disk)))
        extended_allowed = true;
    else
        extended_allowed = false;

    if (free_space->type & PED_PARTITION_LOGICAL)
        logical_freespace = true;
    else if (free_space->type & PED_PARTITION_EXTENDED)
        logical_freespace = true;
    else
        logical_freespace = false;

    combo->insertItem(0, i18n("Primary"));
    combo->insertItem(1, i18n("Extended"));

    if (logical_freespace) {
        combo->insertItem(2, i18n("Logical"));
        combo->setEnabled(false);
        combo->setCurrentIndex(2);
    } else if (!extended_allowed) {
        combo->setEnabled(false);
        combo->setCurrentIndex(0);
    }

    return combo;
}

bool PartitionAddDialog::hasInitialErrors()
{
    PedDisk *const disk = m_partition->getPedPartition()->disk;
    const unsigned ped_type = m_partition->getPedType();
    const bool logical_freespace = (ped_type & PED_PARTITION_FREESPACE) && (ped_type & PED_PARTITION_LOGICAL);
    const int count = ped_disk_get_primary_partition_count(disk);
    const int max_count = ped_disk_get_max_primary_partition_count(disk);

    if (count >= max_count  && (!(logical_freespace || (ped_type & PED_PARTITION_EXTENDED)))) {
        KMessageBox::error(0, i18n("This disk already has %1 primary partitions, the maximum", count));
        return true;
    } else if ((ped_type & PED_PARTITION_EXTENDED) && (!m_partition->isEmpty())) {
        KMessageBox::error(0, i18n("This should not happen. Try selecting the freespace and not the partiton itself"));
        return true;
    }

    return false;
}

bool PartitionAddDialog::bailout()
{
    return m_bailout;
}
