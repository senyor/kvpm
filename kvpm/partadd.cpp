/*
 *
 *
 * Copyright (C) 2009, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
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

#include "pedexceptions.h"
#include "storagepartition.h"

#include <KComboBox>
#include <KConfigSkeleton>
#include <KGlobal>
#include <KLocale>
#include <KMessageBox>

#include <QDebug>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>



//   The "partition" we get here is usually a ped pointer to a the freespace
//   between partitions. It can also be an *empty* extended partition however.


PartitionAddDialog::PartitionAddDialog(StoragePartition *const partition, QWidget *parent)
    : PartitionDialogBase(partition, parent),
      m_storage_partition(partition)
{
    if (!(m_bailout = hasInitialErrors())) {
        insertWidget(buildTypeWidget());

        validateChange();
        
        connect(this, SIGNAL(changed()),
                this, SLOT(validateChange()));
        
        connect(this, SIGNAL(okClicked()),
                this, SLOT(commitPartition()));
    }
}

void PartitionAddDialog::validateChange()
{
    const PedSector sector_size = getSectorSize();
    const PedSector ONE_MIB  = 0x100000 / sector_size;   // sectors per megabyte
    const long long offset = getNewOffset();
    const long long size   = getNewSize();

    updateGraphicAndLabels();

    if ((offset <= getMaxSize() - size) && (size <= getMaxSize() - offset) &&
        (size >= 2 * ONE_MIB) && (isValid())) {
        enableButtonOk(true);
    } else {
        enableButtonOk(false);
    }
}

QGroupBox* PartitionAddDialog::buildTypeWidget()
{
    QGroupBox *const group = new QGroupBox(this);
    QVBoxLayout *const layout = new QVBoxLayout();
    layout->addSpacing(10);

    QLabel *const type_label = new QLabel(i18n("Select type: "));
    QHBoxLayout *const type_layout = new QHBoxLayout;
    m_type_combo = new KComboBox;
    PedPartition *const free_space = getPedPartition();

    bool in_extended;    // true if we are inside an extended partition
    bool extended_ok;    // true if we can create an extended partition here

    // check to see if partition table supports extended
    // partitions and if it already has one

    PedDisk *const disk  = getPedPartition()->disk;

    if ((PED_DISK_TYPE_EXTENDED & disk->type->features) && (!ped_disk_extended_partition(disk)))
        extended_ok = true;
    else
        extended_ok = false;

    if (free_space->type & PED_PARTITION_LOGICAL)
        in_extended = true;
    else if (free_space->type & PED_PARTITION_EXTENDED)
        in_extended = true;
    else
        in_extended = false;

    m_type_combo->insertItem(0, i18n("Primary"));
    m_type_combo->insertItem(1, i18n("Extended"));

    if (in_extended) {
        m_type_combo->insertItem(2, i18n("Logical"));
        m_type_combo->setEnabled(false);
        m_type_combo->setCurrentIndex(2);
    } else if (!extended_ok) {
        m_type_combo->setEnabled(false);
        m_type_combo->setCurrentIndex(0);
    }

    type_label->setBuddy(m_type_combo);
    type_layout->addWidget(type_label);
    type_layout->addWidget(m_type_combo);
    type_layout->addStretch();
    layout->addLayout(type_layout);
    layout->addSpacing(10);

    group->setLayout(layout);

    return group;
}

bool PartitionAddDialog::bailout()
{
    return m_bailout;
}

void PartitionAddDialog::commitPartition()
{
    const PedSector ONE_MIB = 0x100000 / getSectorSize();   // sectors per megabyte
    PedDevice *const device = getPedPartition()->disk->dev;
    PedDisk   *const disk   = getPedPartition()->disk;
    const PedSector max_start = getMaxStart();
    const PedSector max_end   = getMaxEnd();
    PedSector first_sector  = max_start + getNewOffset();
    PedSector last_sector   = first_sector + getNewSize() - 1;
    PedPartitionType type;

    if (first_sector < max_start)
        first_sector = max_start;

    if (last_sector > max_end)
        last_sector = max_end;

    if ((1 + last_sector - first_sector) < (2 * ONE_MIB)) {
        KMessageBox::sorry(0, i18n("Partitions less than two MiB are not supported"));
        return;
    }

    PedAlignment *const start_align = ped_alignment_new(0, ONE_MIB);
    PedAlignment *const end_align   = ped_alignment_new(-1, ONE_MIB);

    PedGeometry  *const max_geom = ped_geometry_new(device, max_start, 1 + max_end - max_start);
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

