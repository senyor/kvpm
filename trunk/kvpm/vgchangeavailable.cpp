/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <QtGui>

#include "vgchangeavailable.h"
#include "volgroup.h"


/* This dialog changes the available status of
   the volume group */

VGChangeAvailableDialog::VGChangeAvailableDialog(VolGroup *volumeGroup, QWidget *parent):
    KDialog(parent)
{

    setWindowTitle("Volume group availability");

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    m_vg_name = volumeGroup->getName();
    QLabel *name_label = new QLabel( "Volume group: <b>" + m_vg_name + "</b>" );
    name_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(name_label);

    m_avail_check = new QCheckBox("Make available?");
    m_avail_check->setChecked(true);
    layout->addWidget(m_avail_check);

}

QStringList VGChangeAvailableDialog::arguments()
{  
    QString allocation_policy;
    QStringList args;

    if(m_avail_check->isChecked()){
	args << "vgchange" 
	     << "--available"
	     << "y";
    }
    else{
	args << "vgchange" 
	     << "--available"
	     << "n";
    }
    
    return args;
}
