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

#include "vgchangealloc.h"

/* This dialog changes the default allocation policy for
   the volume group */

VGChangeAllocDialog::VGChangeAllocDialog(QString volumeGroupName, QWidget *parent):
    KDialog(parent),
    m_vg_name(volumeGroupName)
{

    setWindowTitle( i18n("Extent allocation policy") );

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    QLabel *name_label = new QLabel( i18n("Volume group: <b>%1</b>").arg(m_vg_name) );
    name_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(name_label);

    QGroupBox *alloc_box = new QGroupBox();
    alloc_box->setTitle( i18n("Extent allocation policy") );
    QVBoxLayout *group_layout = new QVBoxLayout();
    alloc_box->setLayout(group_layout);
    layout->addWidget(alloc_box);
    
    m_normal     = new QRadioButton( i18n("Normal") );
    m_contiguous = new QRadioButton( i18n("Contiguous") );
    m_anywhere   = new QRadioButton( i18n("Anwhere") );
    m_cling      = new QRadioButton( i18n("Cling") );    
    m_normal->setChecked(true);
    group_layout->addWidget(m_normal);
    group_layout->addWidget(m_contiguous);
    group_layout->addWidget(m_anywhere);
    group_layout->addWidget(m_cling);  
}

QStringList VGChangeAllocDialog::arguments()
{  
    QString allocation_policy;
    QStringList args;

    if(m_contiguous->isChecked())
	allocation_policy = "contiguous";
    else if(m_anywhere->isChecked())
	allocation_policy = "anywhere";
    else if(m_cling->isChecked())
	allocation_policy = "cling";
    else
	allocation_policy = "normal";

    args << "vgchange" 
	 << "--alloc"
	 << allocation_policy
	 << m_vg_name;

    return args;
}