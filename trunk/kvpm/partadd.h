/*
 *
 *
 * Copyright (C) 2009, 2011, 2012, 2013, 2016 Benjamin Scott   <benscott@nwlink.com>
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

#include "partbase.h"

class QComboBox;
class QGroupBox;

class StoragePartition;


class PartitionAddDialog : public PartitionDialogBase
{
    Q_OBJECT

    StoragePartition *m_storage_partition;
    PedConstraint    *m_ped_constraints;

    bool m_bailout;

    QComboBox *m_type_combo;

    void getMaximumPartition(PedSector &start, PedSector &end, PedSector &sectorSize);
    QGroupBox *buildTypeWidget();

public:
    explicit PartitionAddDialog(StoragePartition *const partition, QWidget *parent = NULL);
    bool bailout();

private slots:
    void commitPartition();
    void validateChange();
};

#endif
