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

#include "processprogress.h"
#include "sizetostring.h"
#include "vgchangeextent.h"
#include "volgroup.h"

bool change_vg_extent(VolGroup *VolumeGroup)
{
    VGChangeExtentDialog dialog(VolumeGroup);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress extent( dialog.arguments(), "Changing extents..." );
        return TRUE;
    }
    else
        return FALSE;
}

VGChangeExtentDialog::VGChangeExtentDialog(VolGroup *VolumeGroup, QWidget *parent) : KDialog(parent)
{
    vg_name = VolumeGroup->getName();
    setWindowTitle("Extent size");

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    QLabel *name_label = new QLabel("Volume group: <b>" + vg_name);
    name_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(name_label);

    QLabel *message = new QLabel( "The extent size is currently: " + 
                                  sizeToString( VolumeGroup->getExtentSize() ) );
    message->setWordWrap(TRUE);
    layout->addWidget(message);

    QHBoxLayout *combo_layout = new QHBoxLayout();
    layout->addLayout(combo_layout);
    extent_size = new QComboBox();
    extent_size->insertItem(0,"1");
    extent_size->insertItem(1,"2");
    extent_size->insertItem(2,"4");
    extent_size->insertItem(3,"8");
    extent_size->insertItem(4,"16");
    extent_size->insertItem(5,"32");
    extent_size->insertItem(6,"64");
    extent_size->insertItem(7,"128");
    extent_size->insertItem(8,"256");
    extent_size->insertItem(9,"512");
    extent_size->setInsertPolicy(QComboBox::NoInsert);
    extent_size->setCurrentIndex(2);
    extent_suffix = new QComboBox();
    extent_suffix->insertItem(0,"KB");
    extent_suffix->insertItem(1,"MB");
    extent_suffix->insertItem(2,"GB");
    extent_suffix->setInsertPolicy(QComboBox::NoInsert);
    extent_suffix->setCurrentIndex(1);
    combo_layout->addWidget(extent_size);
    combo_layout->addWidget(extent_suffix);
}

QStringList VGChangeExtentDialog::arguments()
{  
    QStringList args;
    
    args << "/sbin/vgchange"
	 << "--physicalextentsize"
	 << extent_size->currentText() + extent_suffix->currentText().at(0)
	 << vg_name;

    return args;
}
