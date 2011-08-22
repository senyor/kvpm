/*
 *
 * 
 * Copyright (C) 2008, 2011 Benjamin Scott   <benscott@nwlink.com>
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

#include <KLocale>
#include <QtGui>

#include "physvol.h"
#include "volgroup.h"

PVChangeDialog::PVChangeDialog(PhysVol *physicalVolume, QWidget *parent):
    KDialog(parent),
    m_pv(physicalVolume)
{

    setWindowTitle( i18n("Change physical volume attributes") );

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout;
    dialog_body->setLayout(layout);

    QString pv_name = m_pv->getName();
    QLabel *label = new QLabel( i18n("<b>%1</b>", pv_name) );
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    QGroupBox *attrib_box = new QGroupBox( i18n("Attributes") );
    QVBoxLayout *attrib_box_layout = new QVBoxLayout;
    attrib_box->setLayout(attrib_box_layout);
    layout->addWidget(attrib_box);
    m_allocation_box = new QCheckBox( i18n("Enable allocation of extents") );
    if( m_pv->isAllocatable() )
        m_allocation_box->setChecked(true);
    attrib_box_layout->addWidget(m_allocation_box);
    m_mda_box = new QCheckBox( i18n("Use metadata areas on this volume") );
    if( m_pv->getMDAUsed() )
        m_mda_box->setChecked(true);
    attrib_box_layout->addWidget(m_mda_box);
    m_uuid_box = new QCheckBox( i18n("Generate new UUID for this volume") );
    m_uuid_box->setChecked(false);
    if( m_pv->getVolGroup()->isActive() )
        m_uuid_box->setEnabled(false);
    attrib_box_layout->addWidget(m_uuid_box);

    m_tags_group = new QGroupBox( i18n("Change tags") );
    m_tags_group->setCheckable(true);
    m_tags_group->setChecked(false);
    layout->addWidget(m_tags_group);
    QHBoxLayout *add_tag_layout = new QHBoxLayout();
    QHBoxLayout *del_tag_layout = new QHBoxLayout();
    QVBoxLayout *tag_group_layout = new QVBoxLayout();
    tag_group_layout->addLayout(add_tag_layout);
    tag_group_layout->addLayout(del_tag_layout);
    m_tags_group->setLayout(tag_group_layout);
    add_tag_layout->addWidget( new QLabel( i18n("Add new tag:")) );
    m_tag_edit = new KLineEdit();
    QRegExp rx("[0-9a-zA-Z_\\.+-]*");
    QRegExpValidator *tag_validator = new QRegExpValidator( rx, m_tag_edit );
    m_tag_edit->setValidator(tag_validator);
    add_tag_layout->addWidget(m_tag_edit);
    del_tag_layout->addWidget( new QLabel( i18n("Remove tag:")) );
    m_deltag_combo = new KComboBox();
    m_deltag_combo->setEditable(false);
    QStringList tags = m_pv->getTags();
    for(int x = 0; x < tags.size(); x++)
        m_deltag_combo->addItem( tags[x] );
    m_deltag_combo->insertItem(0, QString(""));
    m_deltag_combo->setCurrentIndex(0);
    del_tag_layout->addWidget(m_deltag_combo);

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

    if( m_allocation_box->isChecked() != m_pv->isAllocatable() )
        enableButtonOk(true);

    if( m_mda_box->isChecked() != m_pv->getMDAUsed() )
        enableButtonOk(true);   

    if( m_uuid_box->isChecked() )
        enableButtonOk(true);

    if( ( m_deltag_combo->currentIndex() || ( !m_tag_edit->text().isEmpty() ) ) && m_tags_group->isChecked() )
        enableButtonOk(true);
}

QStringList PVChangeDialog::arguments()
{
    QStringList args;

    args << "pvchange";
	 
    if( m_allocation_box->isChecked() && !m_pv->isAllocatable() )
	args << "--allocatable" << "y";
    else if( !m_allocation_box->isChecked() && m_pv->isAllocatable() )
	args << "--allocatable" << "n";

    if( m_mda_box->isChecked() && !m_pv->getMDAUsed() )
        args << "--metadataignore" << "n";
    else if( !m_mda_box->isChecked() && m_pv->getMDAUsed() )
        args << "--metadataignore" << "y";

    if( m_uuid_box->isChecked() )
        args << "--uuid";

    if( m_tags_group->isChecked() ){
        if( m_deltag_combo->currentIndex() )
            args << "--deltag" << m_deltag_combo->currentText();
        if( !m_tag_edit->text().isEmpty() )
            args << "--addtag" << m_tag_edit->text();
    }


    args << m_pv->getName();
  
    return args;
}
