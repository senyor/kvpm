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

#include "processprogress.h"
#include "vgchangelv.h"
#include "volgroup.h"

/* This dialog sets a limit on the number of
   logical volumes a group may contain. The
   group may also be unlimited with lvm2 format */


bool change_vg_lv(VolGroup *VolumeGroup)
{
    VGChangeLVDialog dialog(VolumeGroup);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress change( dialog.arguments() );
	return TRUE;
    }
    else
	return FALSE;
}


VGChangeLVDialog::VGChangeLVDialog(VolGroup *VolumeGroup, QWidget *parent) :
    KDialog(parent),
    vg(VolumeGroup)
{
    setWindowTitle("Logical volume limit");
    vg_name = vg->getName();

// We don't want the limit set to less than the number already in existence!

    int lv_count = vg->getLogVolCount();
    if(lv_count <= 0)
	lv_count = 1;

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    QLabel *name_label = new QLabel("Volume group: <b>" + vg_name);
    name_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(name_label);
    
    limit_lvs = new QGroupBox("Maximum logical volumes");
    QVBoxLayout *groupbox_layout = new QVBoxLayout();
    limit_lvs->setLayout(groupbox_layout);

    QLabel *message_label  = new QLabel();
    message_label->setWordWrap(TRUE);
    layout->addWidget(message_label);
    QLabel *current_limit_label = new QLabel;
    layout->addWidget(current_limit_label);
    if( vg->getLogVolMax() )
	current_limit_label->setText( QString( "Current limit: %1" ).arg( vg->getLogVolMax() ) );
    else
	current_limit_label->setText( QString("Current limit: unlimited") );
    
    max_lvs = new QSpinBox();
    groupbox_layout->addWidget(max_lvs);

    if(vg->getFormat() == "lvm1"){
	message_label->setText( (QString) "This volume group is in lvm1 format. Unless you" +
	                       " have a reason to set the limit lower it is normally best" +
	                       " to leave the limit at the maximum allowed: 255." );
	
	max_lvs->setEnabled(TRUE);
	max_lvs->setRange(lv_count, 255);
	max_lvs->setValue(255);
    }
    else{
	message_label->setText( (QString) "This volume group is in lvm2 format. Unless you" +
	                       " have a reason to limit the maximum logical volumes it is" +
	                       " normally best to leave them unlimited" );
	
	limit_lvs->setCheckable(TRUE);
	limit_lvs->setChecked(FALSE);
	limit_lvs->setEnabled(TRUE);
	max_lvs->setMinimum(lv_count);
	max_lvs->setRange(lv_count, 32767); // does anyone need more than 32 thousand?
    }

    layout->addWidget(limit_lvs);
}

QStringList VGChangeLVDialog::arguments()
{
    QStringList args;
    
    args << "/sbin/vgchange"
	 << "--logicalvolume";

    if(max_lvs->isEnabled())
	args << QString( "%1" ).arg( max_lvs->value() );
    else
	args << QString( "%1" ).arg( 0 );           // unlimited

    args << vg_name;

    return args;
}
