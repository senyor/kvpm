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

#include "storagepartition.h"

class QDoubleValidator;
class PartAddGraphic;

bool add_partition(StoragePartition *partition);


class PartitionAddDialog : public KDialog
{
Q_OBJECT

    StoragePartition *m_partition;    
    PedConstraint    *m_ped_constraints;
    PedDisk          *m_ped_disk;

    PedSector m_ped_start_sector,   // first available sector of free space
              m_ped_end_sector,     // last available sector of free space 
              m_ped_sector_length;  // sectors available of free space 

    long long m_ped_sector_size;    // bytes per logical sector
    long long m_partition_sectors;  // proposed size of partition
    long long m_excluded_sectors;   // proposed excluded sectors of partition

    PartAddGraphic *m_display_graphic; // The color bar that shows the relative
                                       // size of the partition graphically
    QSpinBox *m_total_size_spin,
             *m_excluded_spin;

    KLineEdit *m_size_edit,
              *m_excluded_edit;

    QDoubleValidator *m_size_validator,
                     *m_excluded_validator;

    KComboBox *m_size_combo,
              *m_type_combo,
              *m_excluded_combo;

    QGroupBox *m_size_group,
              *m_excluded_group;

    QLabel    *m_unexcluded_label,  // Space left for new partition
              *m_remaining_label,
              *m_preceding_label;

    bool validatePartitionSize(QString size);
    bool validateExcludedSize(QString size);
    void updatePartition();
    long long convertSizeToSectors(int index, double size);

public:
    PartitionAddDialog(StoragePartition *partition, QWidget *parent = 0);

private slots:
    void adjustSizeEdit(int percentage);
    void adjustSizeCombo(int index);
    void validate();
    void adjustExcludedEdit(int percentage);
    void adjustExcludedCombo(int index);
    void commitPartition();
    void clearExcludedGroup(bool on);
};

#endif
