/*
 *
 * 
 * Copyright (C) 2009, 2011 Benjamin Scott   <benscott@nwlink.com>
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

#include <QFrame>
#include <KDialog>
#include <KLineEdit>
#include <KComboBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>


class QDoubleValidator;
class PartAddGraphic;
class DualSelectorBox;
class StoragePartition;


bool add_partition(StoragePartition *partition);


class PartitionAddDialog : public KDialog
{
Q_OBJECT

    StoragePartition *m_partition;    
    PedConstraint    *m_ped_constraints;
    PedDisk          *m_ped_disk;
    DualSelectorBox  *m_dual_selector;

    bool m_use_si_units;

    PedSector m_max_part_start,   // first available sector of free space
              m_max_part_end,     // last available sector of free space 
              m_max_part_size;    // sectors available of free space 

    long long m_sector_size;    // bytes per logical sector

    PartAddGraphic *m_display_graphic; // The color bar that shows the relative
                                       // size of the partition graphically

    QLabel    *m_unexcluded_label,  // Space left for new partition
              *m_remaining_label,
              *m_preceding_label;

    KComboBox *m_type_combo;

    void updatePartition();
    long long convertSizeToSectors(int index, double size);
    void getMaximumPartition();

public:
    explicit PartitionAddDialog(StoragePartition *partition, QWidget *parent = 0);

private slots:
    void commitPartition();
    void validateChange();
};

#endif
