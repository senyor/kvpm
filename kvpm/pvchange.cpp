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
#include "pvchange.h"


PVChangeDialog::PVChangeDialog(QString PhysicalVolumePath, QWidget *parent):KDialog(parent)
{
    path = PhysicalVolumePath;

    setWindowTitle(tr("Change physical volume attributes"));
    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout;
    dialog_body->setLayout(layout);

    QLabel *label = new QLabel("Physical volume: " + path);
    layout->addWidget(label);
    QGroupBox *attrib_box = new QGroupBox("Attributes");
    QVBoxLayout *attrib_box_layout = new QVBoxLayout;
    attrib_box->setLayout(attrib_box_layout);
    layout->addWidget(attrib_box);
    allocation_box = new QCheckBox("Enable allocation of extents");
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

    args << path;
  
    return args;
}
