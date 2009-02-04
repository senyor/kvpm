/*
 *
 * 
 * Copyright (C) 2009 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef PARTMOVERESIZE_H
#define PARTMOVERESIZE_H

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

bool moveresize_partition(StoragePartition *partition);


class PartitionMoveResizeDialog : public KDialog
{
Q_OBJECT

    StoragePartition *m_old_storage_part;    
    PedDisk          *m_ped_disk;
    PedPartition     *m_current_part; // The partition on the disk now

    PedSector m_min_shrink_size;     // Minimum size of the fs after shrinking
    long long m_ped_sector_size;     // bytes per logical sector
    long long m_new_part_size;       // proposed size of partition
    long long m_new_part_start;      // start of new partition in sectors

    long long m_old_part_size;       // original size of partition
    long long m_old_part_start;      // start of original partition in sectors

    long long m_max_part_start;      // start of biggest possible partition
    long long m_max_part_end;        // end of largest possible partition

    PartAddGraphic *m_display_graphic; // The color bar that shows the relative
                                       // size of the partition graphically
    QSpinBox *m_size_spin,
             *m_offset_spin;

    KLineEdit *m_size_edit,
              *m_offset_edit;

    QDoubleValidator *m_size_validator,
                     *m_offset_validator;

    KComboBox *m_size_combo,
              *m_preceding_combo;

    QGroupBox *m_size_group,
              *m_offset_group;

    QCheckBox *m_align64_check;

    QLabel    *m_unexcluded_label,  // Space left for new partition
              *m_remaining_label,
              *m_preceding_label;

    bool m_logical;      // Are we a primary or logical partition?

    void setup();
    void resetOkButton();
    long long convertSizeToSectors(int index, double size);
    long long shrinkfs(PedSector length);
    long long getMinShrinkSize();
    long long getFsBlockSize();
    bool growfs();
    bool movefs(long long from_start, long long to_start, long long length);
    bool shrinkPartition();
    bool growPartition();
    bool movePartition();
    void resetDisplayGraphic();

public:
    PartitionMoveResizeDialog(StoragePartition *partition, QWidget *parent = 0);

private slots:
    void adjustSizeEdit(int percentage);
    void adjustSizeCombo(int index);
    void validateVolumeSize(QString size);
    void adjustPrecedingEdit(int percentage);
    void adjustPrecedingCombo(int index);
    void validatePrecedingSize(QString size);
    void commitPartition();
    void resetOffsetGroup(bool on);
    void resetSizeGroup(bool on);
};

#endif
