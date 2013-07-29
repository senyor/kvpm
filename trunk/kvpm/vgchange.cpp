/*
 *
 *
 * Copyright (C) 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "vgchange.h"

#include "allocationpolicy.h"
#include "logvol.h"
#include "misc.h"
#include "processprogress.h"
#include "volgroup.h"

#include <KComboBox>
#include <KIntSpinBox>
#include <KLocale>
#include <KSeparator>

#include <QCheckBox>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>



VGChangeDialog::VGChangeDialog(VolGroup *const group, QWidget *parent)
    : KvpmDialog(parent),
      m_vg(group)
{
    setCaption(i18n("Change Volume Group Attributes"));

    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    QLabel *const name_label = new QLabel(i18n("<b>Change volume group: %1</b>", m_vg->getName()));
    name_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(name_label);
    layout->addSpacing(5);

    QGroupBox *const misc_box = new QGroupBox();
    QVBoxLayout *const misc_layout = new QVBoxLayout();
    misc_box->setLayout(misc_layout);
    layout->addWidget(misc_box);
    m_resize = new QCheckBox(i18n("Allow physical volume addition and removal"));
    m_resize->setChecked(m_vg->isResizable());
    misc_layout->addWidget(m_resize);
    m_clustered = new QCheckBox(i18n("Cluster aware"));
    m_clustered->setChecked(m_vg->isClustered());
    misc_layout->addWidget(m_clustered);
    m_refresh = new QCheckBox(i18n("Refresh metadata"));
    misc_layout->addWidget(m_refresh);

    m_uuid = new QCheckBox(i18n("Generate new UUID fo group"));
    if (m_vg->isActive())
        m_uuid->setEnabled(false);
    misc_layout->addWidget(m_uuid);

    AllocationPolicy const policy = m_vg->getPolicy();
    m_policy_combo = new PolicyComboBox(policy);
    QHBoxLayout *const policy_layout = new QHBoxLayout();
    policy_layout->addWidget(m_policy_combo);
    policy_layout->addStretch();
    misc_layout->addLayout(policy_layout);

    m_extent_size_combo = new KComboBox();
    m_extent_size_combo->insertItem(0, "1");
    m_extent_size_combo->insertItem(1, "2");
    m_extent_size_combo->insertItem(2, "4");
    m_extent_size_combo->insertItem(3, "8");
    m_extent_size_combo->insertItem(4, "16");
    m_extent_size_combo->insertItem(5, "32");
    m_extent_size_combo->insertItem(6, "64");
    m_extent_size_combo->insertItem(7, "128");
    m_extent_size_combo->insertItem(8, "256");
    m_extent_size_combo->insertItem(9, "512");
    m_extent_size_combo->setInsertPolicy(QComboBox::NoInsert);
    m_extent_size_combo->setCurrentIndex(2);

    m_extent_suffix_combo = new KComboBox();
    m_extent_suffix_combo->insertItem(0, "KiB");
    m_extent_suffix_combo->insertItem(1, "MiB");
    m_extent_suffix_combo->insertItem(2, "GiB");
    m_extent_suffix_combo->setInsertPolicy(QComboBox::NoInsert);
    m_extent_suffix_combo->setCurrentIndex(1);

    uint64_t current_extent_size = m_vg->getExtentSize() / 1024;

    if (current_extent_size <= 512)
        m_extent_suffix_combo->setCurrentIndex(0);
    else if (((current_extent_size /= 1024)) <= 512)
        m_extent_suffix_combo->setCurrentIndex(1);
    else {
        m_extent_suffix_combo->setCurrentIndex(2);
        current_extent_size /= 1024;
    }

    for (int x = 0; x < 10; x++) {
        if (current_extent_size == m_extent_size_combo->itemText(x).toULongLong())
            m_extent_size_combo->setCurrentIndex(x);
    }

    QHBoxLayout *const combo_layout = new QHBoxLayout();
    combo_layout->addWidget(new QLabel(i18n("Extent size:")));
    combo_layout->addWidget(m_extent_size_combo);
    combo_layout->addWidget(m_extent_suffix_combo);
    combo_layout->addStretch();
    misc_layout->addLayout(combo_layout);

    connect(m_extent_suffix_combo, SIGNAL(activated(int)),
            this, SLOT(limitExtentSize(int)));

    QHBoxLayout *const middle_layout = new QHBoxLayout();
    layout->addLayout(middle_layout);

    m_available_box = new QGroupBox("Change group availability");
    QVBoxLayout *const available_layout = new QVBoxLayout();
    m_available_yes = new QRadioButton(i18n("Make all logical volumes available"));
    m_available_no = new QRadioButton(i18n("Make all logical volumes unavailable"));
    m_available_yes->setChecked(true);
    available_layout->addWidget(m_available_yes);
    available_layout->addWidget(m_available_no);
    m_available_box->setLayout(available_layout);
    m_available_box->setCheckable(true);
    m_available_box->setChecked(false);
    middle_layout->addWidget(m_available_box);

    m_polling_box = new QGroupBox("Change group polling");
    QVBoxLayout *polling_layout = new QVBoxLayout();
    m_polling_yes = new QRadioButton(i18n("Start polling"));
    m_polling_no = new QRadioButton(i18n("Stop polling"));
    m_polling_yes->setChecked(true);
    polling_layout->addWidget(m_polling_yes);
    polling_layout->addWidget(m_polling_no);
    m_polling_box->setLayout(polling_layout);
    m_polling_box->setCheckable(true);
    m_polling_box->setChecked(false);
    middle_layout->addWidget(m_polling_box);

    LogVolList lv_list = m_vg->getLogicalVolumes();

    for (int x = lv_list.size() - 1; x >= 0 ; x--) { // replace snap containers with first level children
        if (lv_list[x]->isSnapContainer()) {
            lv_list.append(lv_list[x]->getChildren());
            lv_list.removeAt(x);
        }
    }
    for (int x = lv_list.size() - 1; x >= 0 ; x--) {
        if (lv_list[x]->isMounted()) {
            m_available_yes->setChecked(true);
            m_available_no->setEnabled(false);
            break;
        }
    }

// We don't want the limit set to less than the number already in existence!

    int lv_count = m_vg->getLvCount();
    if (lv_count <= 0)
        lv_count = 1;

    m_limit_box = new QGroupBox(i18n("Change maximum limit for number of volumes"));
    QVBoxLayout *const limit_groupbox_layout = new QVBoxLayout();
    m_limit_box->setLayout(limit_groupbox_layout);

    QLabel *const lv_limit_label = new QLabel(i18n("Logical volumes"));
    QLabel *const pv_limit_label = new QLabel(i18n("Physical volumes"));

    QString lv_current_max;
    QString pv_current_max;

    if (m_vg->getLvMax() > 0)
        lv_current_max = QString("%1").arg(m_vg->getLvMax());
    else
        lv_current_max = QString("unlimited");

    if (m_vg->getPvMax() > 0)
        pv_current_max = QString("%1").arg(m_vg->getPvMax());
    else
        pv_current_max = QString("unlimited");

    QLabel *const lv_min_label = new QLabel(i18n("Currently: %1 Minimum: %2", lv_current_max, m_vg->getLvCount()));
    QLabel *const pv_min_label = new QLabel(i18n("Currently: %1 Minimum: %2", pv_current_max, m_vg->getPvCount()));

    QHBoxLayout *const lv_layout = new QHBoxLayout();
    QHBoxLayout *const pv_layout = new QHBoxLayout();

    m_max_pvs_spin = new KIntSpinBox();
    m_max_lvs_spin = new KIntSpinBox();
    lv_limit_label->setBuddy(m_max_lvs_spin);
    pv_limit_label->setBuddy(m_max_pvs_spin);

    if (m_vg->getFormat() == "lvm1") {
        m_max_lvs_spin->setEnabled(true);
        m_max_lvs_spin->setRange(lv_count, 255);
    } else {
        m_limit_box->setCheckable(true);
        m_limit_box->setChecked(false);
        m_limit_box->setEnabled(true);
        m_max_lvs_spin->setSpecialValueText(i18n("unlimited"));
        m_max_lvs_spin->setRange(0, 32767); // does anyone need more than 32 thousand?
    }
    m_max_lvs_spin->setValue(m_vg->getLvMax());

    lv_layout->addWidget(lv_limit_label);
    lv_layout->addWidget(m_max_lvs_spin);
    lv_layout->addWidget(lv_min_label);
    lv_layout->addStretch();
    pv_layout->addWidget(pv_limit_label);
    pv_layout->addWidget(m_max_pvs_spin);
    pv_layout->addWidget(pv_min_label);
    pv_layout->addStretch();

    limit_groupbox_layout->addLayout(lv_layout);
    limit_groupbox_layout->addLayout(pv_layout);

// We don't want the limit set to less than the number already in existence!

    int pv_count = m_vg->getPvCount();
    if (pv_count <= 0)
        pv_count = 1;

    if (m_vg->getFormat() == "lvm1") {
        m_max_pvs_spin->setEnabled(true);
        m_max_pvs_spin->setRange(pv_count, 255);
    } else {
        m_max_pvs_spin->setSpecialValueText(i18n("unlimited"));
        m_max_pvs_spin->setRange(0, 32767); // does anyone need more than 32 thousand?
    }
    m_max_pvs_spin->setValue(m_vg->getPvMax());

    layout->addWidget(m_limit_box);

    connect(m_available_yes, SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_available_no,  SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_polling_yes,   SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_polling_no,    SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_available_box, SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_polling_box,   SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_resize,        SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_clustered,     SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_refresh,       SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_uuid,          SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_limit_box ,    SIGNAL(toggled(bool)),     this, SLOT(resetOkButton()));
    connect(m_max_lvs_spin,  SIGNAL(valueChanged(int)), this, SLOT(resetOkButton()));
    connect(m_max_pvs_spin,  SIGNAL(valueChanged(int)), this, SLOT(resetOkButton()));
    connect(m_extent_size_combo,   SIGNAL(currentIndexChanged(int)), this, SLOT(resetOkButton()));
    connect(m_extent_suffix_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(resetOkButton()));
    connect(m_policy_combo,  SIGNAL(policyChanged(AllocationPolicy)), this, SLOT(resetOkButton()));

    enableButtonOk(false);
}

void VGChangeDialog::commit()
{
    ProcessProgress vgchange(arguments());
}

QStringList VGChangeDialog::arguments()
{
    QStringList args = QStringList() << "vgchange";

    if (m_vg->getPolicy() != m_policy_combo->getPolicy())
        args << "--alloc" << policyToString(m_policy_combo->getPolicy());

    if (m_available_box->isChecked()) {
        if (m_available_yes->isChecked())
            args << "--available" << "y";
        else
            args << "--available" << "n";
    }

    if (m_limit_box->isChecked()) {
        if (m_max_lvs_spin->value() != m_vg->getLvMax())
            args << "--logicalvolume" << QString("%1").arg(m_max_lvs_spin->value());

        if (m_max_pvs_spin->value() != m_vg->getPvMax())
            args << "--maxphysicalvolumes" << QString("%1").arg(m_max_pvs_spin->value());
    }

    if (m_resize->isChecked() != m_vg->isResizable()) {
        if (m_resize->isChecked())
            args << "--resizeable" << "y";
        else
            args << "--resizeable" << "n";
    }

    if (m_clustered->isChecked() != m_vg->isClustered()) {
        if (m_clustered->isChecked())
            args << "--clustered" << "y";
        else
            args << "--clustered" << "n";
    }

    if (m_refresh->isChecked())
        args << "--refresh";

    if (m_uuid->isChecked())
        args << "--uuid";

    uint64_t new_extent_size = m_extent_size_combo->currentText().toULongLong();

    new_extent_size *= 1024;
    if (m_extent_suffix_combo->currentIndex() > 0)
        new_extent_size *= 1024;
    if (m_extent_suffix_combo->currentIndex() > 1)
        new_extent_size *= 1024;

    if (new_extent_size != m_vg->getExtentSize()) {
        args << "--physicalextentsize" << QString("%1b").arg(new_extent_size);
    }

    if (m_polling_box->isChecked()) {
        if (m_polling_yes->isChecked())
            args << "--poll" << "y";
        else
            args << "--poll" << "n";
    }

    args << m_vg->getName();
    return args;
}

void VGChangeDialog::resetOkButton()
{

    if (arguments().size() > 2)
        enableButtonOk(true);
    else
        enableButtonOk(false);

    if (m_limit_box->isChecked()) {
        if ((m_max_lvs_spin->value() < m_vg->getLvCount()) && (m_max_lvs_spin->value() != 0))
            enableButtonOk(false);

        if ((m_max_pvs_spin->value() < m_vg->getPvCount()) && (m_max_pvs_spin->value() != 0))
            enableButtonOk(false);
    }
}

void VGChangeDialog::limitExtentSize(int index)
{
    int extent_index;

    if (index > 1) {  // Gigabytes selected as suffix, more than 2Gib forbidden
        if (m_extent_size_combo->currentIndex() > 2)
            m_extent_size_combo->setCurrentIndex(0);
        m_extent_size_combo->setMaxCount(2);
    } else {
        extent_index = m_extent_size_combo->currentIndex();
        m_extent_size_combo->setMaxCount(10);
        m_extent_size_combo->setInsertPolicy(QComboBox::InsertAtBottom);
        m_extent_size_combo->insertItem(3, "4");
        m_extent_size_combo->insertItem(3, "8");
        m_extent_size_combo->insertItem(4, "16");
        m_extent_size_combo->insertItem(5, "32");
        m_extent_size_combo->insertItem(6, "64");
        m_extent_size_combo->insertItem(7, "128");
        m_extent_size_combo->insertItem(8, "256");
        m_extent_size_combo->insertItem(9, "512");
        m_extent_size_combo->setInsertPolicy(QComboBox::NoInsert);
        m_extent_size_combo->setCurrentIndex(extent_index);
    }
}
