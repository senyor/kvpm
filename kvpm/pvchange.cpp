/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
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


PVChangeDialog::PVChangeDialog(QString physicalVolumePath, QWidget *parent):
    KDialog(parent),
    m_pv_path(physicalVolumePath)

{

    setWindowTitle( i18n("Change physical volume attributes") );

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout;
    dialog_body->setLayout(layout);

    QLabel *label = new QLabel( i18n("Physical volume: %1").arg(m_pv_path) );
    layout->addWidget(label);
    QGroupBox *attrib_box = new QGroupBox( i18n("Attributes") );
    QVBoxLayout *attrib_box_layout = new QVBoxLayout;
    attrib_box->setLayout(attrib_box_layout);
    layout->addWidget(attrib_box);
    allocation_box = new QCheckBox( i18n("Enable allocation of extents") );
    attrib_box_layout->addWidget(allocation_box);

}

QStringList PVChangeDialog::arguments()
{
    QStringList args;

    args << "pvchange"
	 << "--allocatable";

    if(allocation_box->isChecked())
	args << "y";
    else
	args << "n";

    args << m_pv_path;
  
    return args;
}
