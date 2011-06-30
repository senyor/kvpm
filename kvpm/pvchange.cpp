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


#include <KLocale>
#include <QtGui>

#include "pvchange.h"
#include "physvol.h"


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
    QLabel *label = new QLabel( i18n("Physical volume: %1").arg(pv_name) );
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
    attrib_box_layout->addWidget(m_uuid_box);
    enableButtonOk(false);

    connect(m_allocation_box,   SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_mda_box,   SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_uuid_box,   SIGNAL(clicked()), this, SLOT(resetOkButton()));
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

    args << m_pv->getName();
  
    return args;
}
