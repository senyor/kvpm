/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Klvm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <QtGui>
#include "vgchangealloc.h"

/* This dialog changes the default allocation policy for
   the volume group */

VGChangeAllocDialog::VGChangeAllocDialog(QString VolGroupName, QWidget *parent):
    KDialog(parent),
    vg_name(VolGroupName)
{

    setWindowTitle("Extent allocation policy");

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    QLabel *name_label = new QLabel( "Volume group: <b>" + vg_name + "</b>" );
    name_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(name_label);

    QGroupBox *alloc_box = new QGroupBox();
    alloc_box->setTitle("Extent allocation policy");
    QVBoxLayout *group_layout = new QVBoxLayout();
    alloc_box->setLayout(group_layout);
    layout->addWidget(alloc_box);
    
    normal = new QRadioButton("Normal");
    normal->setChecked(TRUE);
    contiguous = new QRadioButton("Contiguous");
    anywhere = new QRadioButton("Anwhere");
    cling = new QRadioButton("Cling");    
    group_layout->addWidget(normal);
    group_layout->addWidget(contiguous);
    group_layout->addWidget(anywhere);
    group_layout->addWidget(cling);  
}

QStringList VGChangeAllocDialog::arguments()
{  
    QString allocation_policy;
    QStringList args;

    if(normal->isChecked())
	allocation_policy = "normal";
    if(contiguous->isChecked())
	allocation_policy = "contiguous";
    if(anywhere->isChecked())
	allocation_policy = "anywhere";
    if(cling->isChecked())
	allocation_policy = "cling";

    args << "/sbin/vgchange" 
	 << "--alloc"
	 << allocation_policy
	 << vg_name;

    return args;
}
