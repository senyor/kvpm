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


#include "partflag.h"

#include "pedexceptions.h"


#include "processprogress.h"
#include "progressbox.h"
#include "storagepartition.h"

#include <KLocale>
#include <KPushButton>

#include <QButtonGroup>
#include <QCheckBox>
#include <QDebug>
#include <QGroupBox>
#include <QLabel>
#include <QStringList>
#include <QVBoxLayout>



PartitionFlagDialog::PartitionFlagDialog(StoragePartition *const partition, QWidget *parent)
    : KDialog(parent),
      m_storage_part(partition)
{
    setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Reset);
    setCaption((i18n("Set Partition Flags")));

    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);


    QVBoxLayout *const layout = new QVBoxLayout();

    QLabel *const label = new QLabel(i18n("<b>Set Partition Flags</b>"));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    layout->addSpacing(10);

    QGroupBox *const check_group = new QGroupBox();
    QVBoxLayout *const check_layout = new QVBoxLayout();
    check_group->setLayout(check_layout);

    m_boot_check   = new QCheckBox("boot");
    m_hidden_check = new QCheckBox("hidden");
    m_legacy_boot_check = new QCheckBox("legacy_boot");
    m_root_check   = new QCheckBox("root");
    m_swap_check   = new QCheckBox("swap");
    m_raid_check   = new QCheckBox("raid");
    m_lvm_check    = new QCheckBox("lvm");
    m_lba_check    = new QCheckBox("lba");
    m_palo_check    = new QCheckBox("palo");
    m_prep_check    = new QCheckBox("prep");
    m_msftres_check = new QCheckBox("msftres");
    m_bios_grub_check = new QCheckBox("bios_grub");
    m_atvrecv_check   = new QCheckBox("atvrecv");
    m_diag_check      = new QCheckBox("diag");
    m_hp_service_check  = new QCheckBox("hp-service");

    m_bg = new QButtonGroup(this);
    m_bg->setExclusive(false);

    check_layout->addWidget(m_boot_check);
    check_layout->addWidget(m_hidden_check);
    check_layout->addWidget(m_legacy_boot_check);
    check_layout->addWidget(m_root_check);
    check_layout->addWidget(m_swap_check);
    check_layout->addWidget(m_raid_check);
    check_layout->addWidget(m_lvm_check);
    check_layout->addWidget(m_lba_check);
    check_layout->addWidget(m_palo_check);
    check_layout->addWidget(m_prep_check);
    check_layout->addWidget(m_msftres_check);
    check_layout->addWidget(m_bios_grub_check);
    check_layout->addWidget(m_atvrecv_check);
    check_layout->addWidget(m_diag_check);
    check_layout->addWidget(m_hp_service_check);

    if (!m_storage_part) {
        m_bailout = true;
    } else if (m_storage_part->isFreespace()) {
        m_bailout = true;
    } else {
        m_bailout = false;
        QStringList available;
        
        const PedPartition *const part = m_storage_part->getPedPartition();
        PedPartitionFlag flag = (PedPartitionFlag)0;
        while ( (flag = ped_partition_flag_next(flag)) ) {
            if( ped_partition_is_flag_available(part, flag) ) {
                available << QString(ped_partition_flag_get_name(flag)).trimmed();
            }
        }

        if (QString(part->disk->type->name).trimmed() == QString("gpt"))
            m_bg->addButton(m_boot_check);

        m_bg->addButton(m_root_check);
        m_bg->addButton(m_swap_check);
        m_bg->addButton(m_raid_check);
        m_bg->addButton(m_lvm_check);
        m_bg->addButton(m_lba_check);
        m_bg->addButton(m_hp_service_check);
        m_bg->addButton(m_palo_check);
        m_bg->addButton(m_prep_check);
        m_bg->addButton(m_msftres_check);
        m_bg->addButton(m_bios_grub_check);
        m_bg->addButton(m_atvrecv_check);
        m_bg->addButton(m_diag_check);

        m_boot_check->setVisible(available.contains("boot"));
        m_root_check->setVisible(available.contains("root"));
        m_swap_check->setVisible(available.contains("swap"));
        m_raid_check->setVisible(available.contains("raid"));
        m_hidden_check->setVisible(available.contains("hidden"));
        m_legacy_boot_check->setVisible(available.contains("legacy_boot"));
        m_lvm_check->setVisible(available.contains("lvm"));
        m_lba_check->setVisible(available.contains("lba"));
        m_hp_service_check->setVisible(available.contains("hp-service"));
        m_palo_check->setVisible(available.contains("palo"));
        m_prep_check->setVisible(available.contains("prep"));
        m_msftres_check->setVisible(available.contains("msftres"));
        m_bios_grub_check->setVisible(available.contains("bios_grub"));
        m_atvrecv_check->setVisible(available.contains("atvrecv"));
        m_diag_check->setVisible(available.contains("diag"));
        
        setChecks();
    }
    
    connect(this, SIGNAL(okClicked()),
            this, SLOT(commit()));

    connect(this, SIGNAL(resetClicked()),
            this, SLOT(setChecks()));

    connect(m_bg, SIGNAL(buttonClicked(QAbstractButton *)),
            this, SLOT(makeExclusive(QAbstractButton *)));

    layout->addWidget(check_group);
    dialog_body->setLayout(layout);
}

bool PartitionFlagDialog::bailout()
{
    return m_bailout;
}


void PartitionFlagDialog::makeExclusive(QAbstractButton *button)
{
    bool checked = button->isChecked();

    const QList<QAbstractButton *> list(m_bg->buttons());

    for (int x = 0; x < list.size(); x++)
        list[x]->setChecked(false);

    button->setChecked(checked);
}


void PartitionFlagDialog::setChecks()
{
    const PedPartition *const part = m_storage_part->getPedPartition();

    m_boot_check->setChecked(ped_partition_get_flag(part, ped_partition_flag_get_by_name("boot")));
    m_root_check->setChecked(ped_partition_get_flag(part, ped_partition_flag_get_by_name("root")));
    m_swap_check->setChecked(ped_partition_get_flag(part, ped_partition_flag_get_by_name("swap")));
    m_legacy_boot_check->setChecked(ped_partition_get_flag(part, ped_partition_flag_get_by_name("legacy_boot")));
    m_hidden_check->setChecked(ped_partition_get_flag(part, ped_partition_flag_get_by_name("hidden")));
    m_raid_check->setChecked(ped_partition_get_flag(part, ped_partition_flag_get_by_name("raid")));
    m_lvm_check->setChecked(ped_partition_get_flag(part, ped_partition_flag_get_by_name("lvm")));
    m_lba_check->setChecked(ped_partition_get_flag(part, ped_partition_flag_get_by_name("lba")));
    m_hp_service_check->setChecked(ped_partition_get_flag(part, ped_partition_flag_get_by_name("hp-service")));
    m_palo_check->setChecked(ped_partition_get_flag(part, ped_partition_flag_get_by_name("palo")));
    m_prep_check->setChecked(ped_partition_get_flag(part, ped_partition_flag_get_by_name("prep")));
    m_msftres_check->setChecked(ped_partition_get_flag(part, ped_partition_flag_get_by_name("msftres")));
    m_bios_grub_check->setChecked(ped_partition_get_flag(part, ped_partition_flag_get_by_name("bios_grub")));
    m_atvrecv_check->setChecked(ped_partition_get_flag(part, ped_partition_flag_get_by_name("atvrecv")));
    m_diag_check->setChecked(ped_partition_get_flag(part, ped_partition_flag_get_by_name("diag")));
}

void PartitionFlagDialog::commit()
{
    PedPartition *const part = m_storage_part->getPedPartition();

    if (m_boot_check->isVisible()) {
        if (m_boot_check->isChecked())
            ped_partition_set_flag(part, PED_PARTITION_BOOT, 1); 
        else
            ped_partition_set_flag(part, PED_PARTITION_BOOT, 0); 
    }

    if (m_root_check->isVisible()) {
        if (m_root_check->isChecked())
            ped_partition_set_flag(part, PED_PARTITION_ROOT, 1); 
        else
            ped_partition_set_flag(part, PED_PARTITION_ROOT, 0); 
    }

    if (m_swap_check->isVisible()) {
        if (m_swap_check->isChecked())
            ped_partition_set_flag(part, PED_PARTITION_SWAP, 1); 
        else
            ped_partition_set_flag(part, PED_PARTITION_SWAP, 0); 
    }

    if (m_hidden_check->isVisible()) {
        if (m_hidden_check->isChecked())
            ped_partition_set_flag(part, PED_PARTITION_HIDDEN, 1);
        else
            ped_partition_set_flag(part, PED_PARTITION_HIDDEN, 0); 
    }

    if (m_legacy_boot_check->isVisible()) {
        if (m_legacy_boot_check->isChecked())
            ped_partition_set_flag(part, ped_partition_flag_get_by_name("legacy_boot"), 1); 
        else
            ped_partition_set_flag(part, ped_partition_flag_get_by_name("legacy_boot"), 0); 
    }

    if (m_raid_check->isVisible()) {
        if (m_raid_check->isChecked())
            ped_partition_set_flag(part, PED_PARTITION_RAID, 1); 
        else
            ped_partition_set_flag(part, PED_PARTITION_RAID, 0); 
    }

    if (m_lvm_check->isVisible()) {
        if (m_lvm_check->isChecked())
            ped_partition_set_flag(part, PED_PARTITION_LVM, 1); 
        else
            ped_partition_set_flag(part, PED_PARTITION_LVM, 0); 
    }

    if (m_lba_check->isVisible()) {
        if (m_lba_check->isChecked())
            ped_partition_set_flag(part, PED_PARTITION_LBA, 1); 
        else
            ped_partition_set_flag(part, PED_PARTITION_LBA, 0); 
    }

    if (m_hp_service_check->isVisible()) {
        if (m_hp_service_check->isChecked())
            ped_partition_set_flag(part, PED_PARTITION_HPSERVICE, 1); 
        else
            ped_partition_set_flag(part, PED_PARTITION_HPSERVICE, 0); 
    }

    if (m_palo_check->isVisible()) {
        if (m_palo_check->isChecked())
            ped_partition_set_flag(part, PED_PARTITION_PALO, 1); 
        else
            ped_partition_set_flag(part, PED_PARTITION_PALO, 0); 
    }

    if (m_prep_check->isVisible()) {
        if (m_prep_check->isChecked())
            ped_partition_set_flag(part, PED_PARTITION_PREP, 1); 
        else
            ped_partition_set_flag(part, PED_PARTITION_PREP, 0); 
    }

    if (m_msftres_check->isVisible()) {
        if (m_msftres_check->isChecked())
            ped_partition_set_flag(part, PED_PARTITION_MSFT_RESERVED, 1); 
        else
            ped_partition_set_flag(part, PED_PARTITION_MSFT_RESERVED, 0); 
    }

    if (m_bios_grub_check->isVisible()) {
        if (m_bios_grub_check->isChecked())
            ped_partition_set_flag(part, PED_PARTITION_BIOS_GRUB, 1); 
        else
            ped_partition_set_flag(part, PED_PARTITION_BIOS_GRUB, 0); 
    }

    if (m_atvrecv_check->isVisible()) {
        if (m_atvrecv_check->isChecked())
            ped_partition_set_flag(part, PED_PARTITION_APPLE_TV_RECOVERY, 1); 
        else
            ped_partition_set_flag(part, PED_PARTITION_APPLE_TV_RECOVERY, 0); 
    }

    if (m_diag_check->isVisible()) {
        if (m_diag_check->isChecked())
            ped_partition_set_flag(part, PED_PARTITION_DIAG, 1); 
        else
            ped_partition_set_flag(part, PED_PARTITION_DIAG, 0); 
    }

    ped_disk_commit(part->disk); 
}
