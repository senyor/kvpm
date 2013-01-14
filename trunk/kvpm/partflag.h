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

#ifndef PARTFLAG_H
#define PARTFLAG_H

#include <parted/parted.h>

#include <KDialog>

class QAbstractButton;
class QButtonGroup;
class QCheckBox;

class StoragePartition;


class PartitionFlagDialog : public KDialog
{
    Q_OBJECT

    QButtonGroup *m_bg;

    QCheckBox *m_legacy_boot_check;
    QCheckBox *m_boot_check;
    QCheckBox *m_hidden_check; 
    QCheckBox *m_raid_check;
    QCheckBox *m_lvm_check; 
    QCheckBox *m_lba_check; 
    QCheckBox *m_hp_service_check; 
    QCheckBox *m_palo_check; 
    QCheckBox *m_prep_check; 
    QCheckBox *m_msftres_check; 
    QCheckBox *m_bios_grub_check; 
    QCheckBox *m_atvrecv_check; 
    QCheckBox *m_diag_check; 
    QCheckBox *m_root_check;
    QCheckBox *m_swap_check;

    StoragePartition *m_storage_part;

    bool m_bailout;

private slots:
    void commit();
    void setChecks();
    void makeExclusive(QAbstractButton *button);

public:
    explicit PartitionFlagDialog(StoragePartition *const partition, QWidget *parent = NULL);
    bool bailout();

};

#endif
