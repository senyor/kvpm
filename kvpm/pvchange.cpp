/*
 *
 *
 * Copyright (C) 2008, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "pvchange.h"

#include <KComboBox>
#include <KLineEdit>
#include <KLocale>

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "physvol.h"
#include "processprogress.h"
#include "volgroup.h"



PVChangeDialog::PVChangeDialog(PhysVol *const physicalVolume, QWidget *parent) :
    KvpmDialog(parent),
    m_pv(physicalVolume)
{
    setCaption(i18n("Change Physical Volume Attributes"));

    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout;
    dialog_body->setLayout(layout);

    QString pv_name = m_pv->getName();
    QLabel *const label = new QLabel(i18n("<b>Changing volume: %1</b>", pv_name));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    layout->addSpacing(5);

    QGroupBox *const attrib_box = new QGroupBox(i18n("Attributes"));
    QVBoxLayout *const attrib_box_layout = new QVBoxLayout;
    attrib_box->setLayout(attrib_box_layout);
    layout->addWidget(attrib_box);
    m_allocation_box = new QCheckBox(i18n("Enable allocation of extents"));
    if (m_pv->isAllocatable())
        m_allocation_box->setChecked(true);
    attrib_box_layout->addWidget(m_allocation_box);
    m_mda_box = new QCheckBox(i18n("Use metadata areas on this volume"));
    if (m_pv->getMdaUsed())
        m_mda_box->setChecked(true);
    attrib_box_layout->addWidget(m_mda_box);
    m_uuid_box = new QCheckBox(i18n("Generate new UUID for this volume"));
    m_uuid_box->setChecked(false);
    if (m_pv->getVg()->isActive())
        m_uuid_box->setEnabled(false);
    attrib_box_layout->addWidget(m_uuid_box);

    m_tags_group = new QGroupBox(i18n("Change tags"));
    m_tags_group->setCheckable(true);
    m_tags_group->setChecked(false);
    layout->addWidget(m_tags_group);
    QHBoxLayout *const add_tag_layout = new QHBoxLayout();
    QHBoxLayout *const del_tag_layout = new QHBoxLayout();
    QVBoxLayout *const tag_group_layout = new QVBoxLayout();
    tag_group_layout->addLayout(add_tag_layout);
    tag_group_layout->addLayout(del_tag_layout);
    m_tags_group->setLayout(tag_group_layout);
    QLabel *const add_tag_label = new QLabel(i18n("Add new tag:"));
    add_tag_layout->addWidget(add_tag_label);
    m_tag_edit = new KLineEdit();
    add_tag_label->setBuddy(m_tag_edit);
    QRegExp rx("[0-9a-zA-Z_\\.+-]*");
    QRegExpValidator *tag_validator = new QRegExpValidator(rx, m_tag_edit);
    m_tag_edit->setValidator(tag_validator);
    add_tag_layout->addWidget(m_tag_edit);
    QLabel *const del_tag_label = new QLabel(i18n("Remove tag:"));
    del_tag_layout->addWidget(del_tag_label);
    m_deltag_combo = new KComboBox();
    del_tag_label->setBuddy(m_deltag_combo);
    m_deltag_combo->setEditable(false);
    const QStringList tags = m_pv->getTags();
    for (int x = 0; x < tags.size(); x++){
        m_deltag_combo->addItem(tags[x]);
    }
    m_deltag_combo->insertItem(0, QString(""));
    m_deltag_combo->setCurrentIndex(0);
    del_tag_layout->addWidget(m_deltag_combo);
    del_tag_layout->addStretch();

    layout->addWidget(m_tags_group);

    enableButtonOk(false);

    connect(m_allocation_box, SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_mda_box,        SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_uuid_box,       SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_tags_group,     SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_deltag_combo,   SIGNAL(currentIndexChanged(int)), this, SLOT(resetOkButton()));
    connect(m_tag_edit,       SIGNAL(userTextChanged(QString)), this, SLOT(resetOkButton()));
}

void PVChangeDialog::resetOkButton()
{
    enableButtonOk(false);

    if (m_allocation_box->isChecked() != m_pv->isAllocatable())
        enableButtonOk(true);

    if (m_mda_box->isChecked() != m_pv->getMdaUsed())
        enableButtonOk(true);

    if (m_uuid_box->isChecked())
        enableButtonOk(true);

    if ((m_deltag_combo->currentIndex() || (!m_tag_edit->text().isEmpty())) && m_tags_group->isChecked())
        enableButtonOk(true);
}

QStringList PVChangeDialog::arguments()
{
    QStringList args = QStringList() << "pvchange";

    if (m_allocation_box->isChecked() && !m_pv->isAllocatable())
        args << "--allocatable" << "y";
    else if (!m_allocation_box->isChecked() && m_pv->isAllocatable())
        args << "--allocatable" << "n";

    if (m_mda_box->isChecked() && !m_pv->getMdaUsed())
        args << "--metadataignore" << "n";
    else if (!m_mda_box->isChecked() && m_pv->getMdaUsed())
        args << "--metadataignore" << "y";

    if (m_uuid_box->isChecked())
        args << "--uuid";

    if (m_tags_group->isChecked()) {
        if (m_deltag_combo->currentIndex())
            args << "--deltag" << m_deltag_combo->currentText();
        if (!m_tag_edit->text().isEmpty())
            args << "--addtag" << m_tag_edit->text();
    }

    args << m_pv->getName();

    return args;
}

void PVChangeDialog::commit()
{
    hide();
    ProcessProgress move(arguments());
    return;
}



