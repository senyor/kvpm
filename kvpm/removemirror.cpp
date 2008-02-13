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

#include "logvol.h"
#include "processprogress.h"
#include "removemirror.h"
#include "volgroup.h"

bool remove_mirror(LogVol *LogicalVolume)
{
    RemoveMirrorDialog dialog(LogicalVolume);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress remove_mirror(dialog.arguments());
        return TRUE;
    }
    else
        return FALSE;
}

RemoveMirrorDialog::RemoveMirrorDialog(LogVol *LogicalVolume, QWidget *parent):KDialog(parent)
{
    lv = LogicalVolume;
    vg = lv->getVolumeGroup();
    logical_volume_name = lv->getFullName();

    setWindowTitle(tr("Add Mirror to Logical Volume"));

    QWidget *dialog_body = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;
    dialog_body->setLayout(layout);
    setMainWidget(dialog_body);
    
    QHBoxLayout *count_layout = new QHBoxLayout();
    QLabel *message = new QLabel("Number of mirrors:");
    count_layout->addWidget(message);

    count_spin = new QSpinBox();
    count_spin->setMinimum(0);
    count_layout->addWidget(count_spin);
    layout->addLayout(count_layout);
}

/* Here we create a string based on all
   the options that the user chose in the
   dialog and feed that to "lvconvert"     
*/

QStringList RemoveMirrorDialog::arguments()
{
    QStringList tempstrings;
    QStringList args;

    args << "/sbin/lvconvert"
	 << "--mirrors" 
	 << QString("%1").arg(count_spin->value()) 
	 << logical_volume_name;
    
    return args;
}



