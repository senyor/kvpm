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

#ifndef PARTBASE_H
#define PARTBASE_H

#include <parted/parted.h>

#include <KDialog>

#include <QWidget>

class QGroupBox;
class QLabel;
class QString;
class QVBoxLayout;

class PartitionGraphic;
class DualSelectorBox;
class StoragePartition;


class PartitionDialogBase : public KDialog
{
    Q_OBJECT

    StoragePartition *m_old_storage_part;
    PedPartition     *m_existing_part; // The partition on the disk now
    QVBoxLayout *m_layout; 
    bool m_use_si_units;
    bool m_is_new;        // we are creating a new partition, not changing an existing one
    bool m_bailout;

    PedSector m_min_shrink_size;   // Minimum size of the fs after shrinking -- in sectors
    PedSector m_sector_size;       // bytes per logical sector
    PedSector m_max_start;         // start of biggest possible partition
    PedSector m_max_end;           // end of largest possible partition
    PedSector m_current_start; 
    PedSector m_current_size; 

    QString m_path;  // the path to the partition under /dev

    PartitionGraphic *m_display_graphic; // The color bar that shows the relative
                                         // size of the partition graphically

    QLabel *m_change_by_label,  // How much are we growing or shrinking the partition?
           *m_move_by_label,    // How much are we moving the partition?
           *m_preceding_label,  // Free space before the proposed partition
           *m_following_label;

    DualSelectorBox *m_dual_selector;

    bool setMaxFreespace(PedSector &start, PedSector &end);
    void setMaxPart(PedSector &start, PedSector &end);
    PedSector setMinSize();
    void buildDialog();

signals:
    void changed();

protected:
    void updateGraphicAndLabels();
    bool isValid();
    bool hasInitialErrors();
    QString getPath();
    PedPartition *getPedPartition();
    bool pedCommitAndWait(PedDisk *const disk);
    PedSector getSectorSize();
    void insertWidget(QWidget *const widget);

    // The following are all in *sectors* -- just in case that isn't obvious
    PedSector getMaxSize();
    PedSector getMaxStart();
    PedSector getMaxEnd();
    PedSector getNewOffset();
    PedSector getMinSize();
    PedSector getNewSize();
    PedSector getCurrentStart();
    PedSector getCurrentSize();

public:
    explicit PartitionDialogBase(StoragePartition *const partition, QWidget *parent = NULL);

};

#endif
