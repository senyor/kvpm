/*
 *
 * 
 * Copyright (C) 2009, 2011 Benjamin Scott   <benscott@nwlink.com>
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

#include <KComboBox>
#include <KDialog>
#include <KLineEdit>

#include <QDoubleValidator>
#include <QEventLoop>
#include <QFrame>
#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>


class PartitionGraphic;
class SizeSelectorBox;
class StoragePartition;


bool moveresize_partition(StoragePartition *partition);

class PartitionMoveResizeDialog : public KDialog
{
Q_OBJECT

    StoragePartition *m_old_storage_part;    
    PedDisk          *m_ped_disk;
    PedPartition     *m_existing_part; // The partition on the disk now

    bool m_use_si_units;

    PedSector m_min_shrink_size;     // Minimum size of the fs after shrinking
    long long m_sector_size;         // bytes per logical sector
    long long m_max_part_start;      // start of biggest possible partition
    long long m_max_part_size;       // size of largest possible partition

    PartitionGraphic *m_display_graphic; // The color bar that shows the relative
                                         // size of the partition graphically

    SizeSelectorBox *m_size_selector,
                    *m_offset_selector;

    QLabel *m_change_by_label,  // How much are we growing or shrinking the partition? 
           *m_preceding_label,  // Free space before the proposed partition
           *m_following_label;

    bool m_logical;      // Are we a logical partition?

    void setup();
    bool movefs(long long from_start, long long to_start, long long length);
    bool shrinkPartition();
    bool growPartition();
    bool movePartition();
    void updateGraphicAndLabels();
    bool pedCommitAndWait(PedDisk *disk);
    void updateAndValidatePartition();
    void getMaximumPartition();

public:
    explicit PartitionMoveResizeDialog(StoragePartition *partition, QWidget *parent = 0);

private slots:
    void commitPartition();
    void resetSelectors();
    void offsetChanged();
    void sizeChanged();

};

#endif
