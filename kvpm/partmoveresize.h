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

#include <KDialog>
#include <KLineEdit>
#include <KComboBox>

#include <QFrame>
#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>
#include <QEventLoop>

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
              *m_offset_combo;

    QGroupBox *m_size_group,
              *m_offset_group;

    QCheckBox *m_align64_check;

    QLabel    *m_remaining_label,
              *m_size_label,
              *m_preceding_label;

    bool m_logical;      // Are we a primary or logical partition?

    void setup();
    void resetOkButton();
    long long convertSizeToSectors(int index, double size);
    long long getMinShrinkSize();
    bool movefs(long long from_start, long long to_start, long long length);
    bool shrinkPartition();
    bool growPartition();
    bool movePartition();
    void resetDisplayGraphic();
    void setSizeLabels();
    bool waitPartitionTableReload();

public:
    PartitionMoveResizeDialog(StoragePartition *partition, QWidget *parent = 0);

private slots:
    void adjustSizeEdit(int percentage);
    void adjustSizeCombo(int index);
    void validateVolumeSize(QString size);
    void adjustOffsetEdit(int percentage);
    void adjustOffsetCombo(int index);
    void validateOffsetSize(QString size);
    void commitPartition();
    void resetOffsetGroup(bool on);
    void resetSizeGroup(bool on);
    void resetPartition();
    void setOffsetSpinMinMax();
    void setSizeSpinMinMax();
    void minimizePartition(bool);
    void maximizePartition(bool);
    void minimizeOffset(bool);
    void maximizeOffset(bool);

};

#endif
