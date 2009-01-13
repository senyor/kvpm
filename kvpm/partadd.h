/*
 *
 * 
 * Copyright (C) 2009 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef PARTADD_H
#define PARTADD_H

#include <parted/parted.h>

#include <KDialog>
#include <QSpinBox>
#include <QCheckBox>

#include "storagepartition.h"

bool add_partition(StoragePartition *partition);


class PartitionAddDialog : public KDialog
{
Q_OBJECT

    StoragePartition *m_partition;    
    PedConstraint    *m_ped_constraints;
    PedDisk          *m_ped_disk;

    PedSector m_ped_start_sector, 
              m_ped_end_sector,
              m_ped_sector_length;

    long long m_ped_sector_size;

    QSpinBox *m_start_sector_spin,
             *m_end_sector_spin;

    QSpinBox *m_start_size_spin,
             *m_end_size_spin,
             *m_total_size_spin;

    QGroupBox *m_size_group,
              *m_sector_group;

    QCheckBox *m_align64_check;

public:
    PartitionAddDialog(StoragePartition *partition, QWidget *parent = 0);
    void commit_partition();

public slots:
    void recalculate(int);
    void sectorGroupBoxAlternate(bool checked);
    void sizeGroupBoxAlternate(bool checked);
    
};

#endif
