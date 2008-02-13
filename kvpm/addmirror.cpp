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
#include "addmirror.h"
#include "logvol.h"
#include "processprogress.h"
#include "volgroup.h"


bool add_mirror(LogVol *LogicalVolume)
{
   AddMirrorDialog dialog(LogicalVolume);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress add_mirror(dialog.arguments(), "Adding Mirror...", TRUE);
        return TRUE;
    }
    else
	return FALSE;
}


AddMirrorDialog::AddMirrorDialog(LogVol *LogicalVolume, QWidget *parent):
    KDialog(parent),
    lv(LogicalVolume)
{
    vg = lv->getVolumeGroup();
    logical_volume_name = lv->getFullName();

    setWindowTitle(tr("Add Mirror to Logical Volume"));

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout;
    dialog_body->setLayout(layout);

    QGroupBox *alloc_box = new QGroupBox("Allocation Policy");
    QVBoxLayout *alloc_box_layout = new QVBoxLayout;
    normal_button     = new QRadioButton("Normal");
    contiguous_button = new QRadioButton("Contiguous");
    anywhere_button   = new QRadioButton("Anywhere");
    inherited_button  = new QRadioButton("Inherited");
    cling_button      = new QRadioButton("Cling");
    normal_button->setChecked(TRUE);
    alloc_box_layout->addWidget(normal_button);
    alloc_box_layout->addWidget(contiguous_button);
    alloc_box_layout->addWidget(anywhere_button);
    alloc_box_layout->addWidget(inherited_button);
    alloc_box_layout->addWidget(cling_button);
    alloc_box->setLayout(alloc_box_layout);
    layout->addWidget(alloc_box);

    QGroupBox *log_box = new QGroupBox("Mirror logging");
    QVBoxLayout *log_box_layout = new QVBoxLayout;
    core_log = new QRadioButton("Memory based logging");
    disk_log = new QRadioButton("Disk based logging");
    disk_log->setChecked(TRUE);
    log_box_layout->addWidget(disk_log);
    log_box_layout->addWidget(core_log);
    log_box->setLayout(log_box_layout);
    layout->addWidget(log_box);
    
    QHBoxLayout *count_layout = new QHBoxLayout();
    QLabel *message = new QLabel("Number of mirrors:");
    count_layout->addWidget(message);

    count_spin = new QSpinBox();
    count_spin->setMinimum(1);
    count_layout->addWidget(count_spin);
    layout->addLayout(count_layout);
}

/* Here we create a string based on all
   the options that the user chose in the
   dialog and feed that to "lvconvert"     
*/

QStringList AddMirrorDialog::arguments()
{
    QStringList tempstrings;
    QStringList args;

    args << "/sbin/lvconvert";
    
    if(core_log->isChecked())
	args << "--corelog";

    args << "--mirrors" 
	 << QString("%1").arg(count_spin->value()) 
	 << logical_volume_name;

    return args;
}
