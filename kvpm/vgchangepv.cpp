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
#include "vgchangepv.h"
#include "volgroup.h"


/* This dialog sets a limit on the number of
   physical volumes a group may contain. The
   group may also be unlimited with lvm2 format */


bool change_vg_pv(VolGroup *VolumeGroup)
{
    VGChangePVDialog dialog(VolumeGroup);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress change( dialog.arguments(), "Changing limits..." );
	return TRUE;
    }
    else
	return FALSE;
}


VGChangePVDialog::VGChangePVDialog(VolGroup *VolumeGroup, QWidget *parent) : KDialog(parent)
{
    vg = VolumeGroup;
    vg_name = vg->getName();

// We don't want the limit set to less than the number already in existence!

    int pv_count = vg->getPhysVolCount();
    if(pv_count <= 0)
	pv_count = 1;

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    QLabel *name_label = new QLabel("Volume group: <b>" + vg_name);
    name_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(name_label);

    limit_pvs = new QGroupBox("Limit maximum physical volumes");
    QVBoxLayout *groupbox_layout = new QVBoxLayout();
    limit_pvs->setLayout(groupbox_layout);

    QLabel *message_label  = new QLabel();
    message_label->setWordWrap(TRUE);
    layout->addWidget(message_label);
    QLabel *current_limit_label = new QLabel;
    layout->addWidget(current_limit_label);
    if( vg->getPhysVolMax() )
	current_limit_label->setText( QString( "Current limit: %1" ).arg( vg->getPhysVolMax() ) );
    else
	current_limit_label->setText( QString("Current limit: unlimited") );
    
    max_pvs = new QSpinBox();
    groupbox_layout->addWidget(max_pvs);

    if(vg->getFormat() == "lvm1"){
	message_label->setText( (QString) "This volume group is in lvm1 format. Unless you" +
	                       " have a reason to set the limit lower, it is normally best" +
	                       " to leave the it at the maximum allowed: 255." );
	
	max_pvs->setEnabled(TRUE);
	max_pvs->setRange(pv_count, 255);
	max_pvs->setValue(255);
    }
    else{
	message_label->setText( (QString) "This volume group is in lvm2 format. Unless you" +
	                       " have a reason to limit the maximum physical volumes it is" +
	                       " normally best to leave them unlimited" );
	
	limit_pvs->setCheckable(TRUE);
	limit_pvs->setChecked(FALSE);
	limit_pvs->setEnabled(TRUE);
	max_pvs->setMinimum(pv_count);
	max_pvs->setRange(pv_count, 32767); // does anyone need more than 32 thousand?
    }

    layout->addWidget(limit_pvs);
}

QStringList VGChangePVDialog::arguments()
{
    QStringList args;
    
    args << "vgchange"
	 << "--maxphysicalvolumes";

    if(max_pvs->isEnabled())
	args << QString( "%1" ).arg( max_pvs->value() );
    else
	args << QString( "%1" ).arg( 0 );           // unlimited

    args << vg_name;

    return args;
}
