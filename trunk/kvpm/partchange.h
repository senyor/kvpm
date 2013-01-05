/*
 *
 *
 * Copyright (C) 2009, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef PARTCHANGESIZE_H
#define PARTCHANGESIZE_H

#include <parted/parted.h>

#include <KDialog>

#include "partbase.h"

class StoragePartition;


class PartitionChangeDialog : public PartitionDialogBase
{
    Q_OBJECT

    StoragePartition *m_old_storage_part;

    bool m_bailout;

    bool movefs(const PedSector from_start, const PedSector to_start, const PedSector length);
    bool shrinkPartition();
    bool growPartition();
    bool movePartition();
    bool continueBackup();
    bool continueResize();
    bool continueBusy();

public:
    explicit PartitionChangeDialog(StoragePartition *const partition, QWidget *parent = NULL);
    bool bailout();

private slots:
    void commitPartition();
    void validateChange();

};

#endif
