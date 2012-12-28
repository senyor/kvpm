/*
 *
 *
 * Copyright (C) 2009, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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

class QFrame;
class KLineEdit;
class KComboBox;
class QGroupBox;
class QCheckBox;
class QLabel;

class QDoubleValidator;
class PartitionGraphic;
class DualSelectorBox;
class StoragePartition;


class PartitionAddDialog : public KDialog
{
    Q_OBJECT

    StoragePartition *m_partition;
    PedConstraint    *m_ped_constraints;
    DualSelectorBox  *m_dual_selector;

    bool m_use_si_units, m_bailout;

    PedSector m_max_part_start,   // first available sector of free space
              m_max_part_end,     // last available sector of free space
              m_max_part_size;    // sectors available of free space

    long long m_sector_size;    // bytes per logical sector

    PartitionGraphic *m_display_graphic; // The color bar that shows the relative
    // size of the partition graphically

    QLabel *m_remaining_label,  // space left past the end of the proposed partition
           *m_preceding_label;  // ditto for the preceding space

    KComboBox *m_type_combo;

    long long convertSizeToSectors(int index, double size);
    void getMaximumPartition(PedSector &start, PedSector &end, PedSector &sectorSize);
    QGroupBox *buildTypeWidget();
    bool hasInitialErrors();

public:
    explicit PartitionAddDialog(StoragePartition *const partition, QWidget *parent = 0);
    bool bailout();

private slots:
    void commitPartition();
    void validateChange();
};

#endif
