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


#include <KMessageBox>
#include <QtGui>

#include "masterlist.h"
#include "processprogress.h"
#include "vgreduce.h"
#include "volgroup.h"

extern MasterList *master_list;

bool reduce_vg(VolGroup *VolumeGroup)
{
    VGReduceDialog dialog(VolumeGroup);
    dialog.exec();

    if(dialog.result() == QDialog::Accepted){
        ProcessProgress reduce( dialog.arguments() );
        return TRUE;
    }

    return FALSE;
}

VGReduceDialog::VGReduceDialog(VolGroup *VolumeGroup, QWidget *parent) : KDialog(parent)
{
    setWindowTitle(tr("Reduce Volume Group"));
    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout;
    dialog_body->setLayout(layout);

    unremovable_pvs_present = FALSE;
    vg_name = VolumeGroup->getName();

    QLabel *label = new QLabel( QString( "Select any of the following physical volumes" ) +
				" to remove them from volume group <b>" +
				vg_name +
				"</b>" );
    label->setWordWrap(TRUE);
    layout->addWidget(label);

    QGroupBox *pv_unused_box = new QGroupBox("Unused physical volumes");
    QVBoxLayout *pv_unused_layout = new QVBoxLayout;
    pv_unused_box->setLayout(pv_unused_layout);
    layout->addWidget(pv_unused_box);

    QGroupBox *pv_used_box = new QGroupBox("In use physical volumes");
    QVBoxLayout *pv_used_layout = new QVBoxLayout;
    pv_used_box->setLayout(pv_used_layout);

    QList<PhysVol *> member_pvs = VolumeGroup->getPhysicalVolumes();
    int pv_count = VolumeGroup->getPhysVolCount(); 
    NoMungeCheck *pv_check = NULL;
    
    for(int x = 0; x < pv_count; x++){
	if( !( member_pvs[x]->getUsed() ) ){                  // only unused pvs can be removed
	    pv_check = new NoMungeCheck( member_pvs[x]->getDeviceName() );
	    pv_check_boxes.append(pv_check);
	    pv_unused_layout->addWidget(pv_check);
	    connect(pv_check, SIGNAL(toggled(bool)), this, SLOT(excludeOneVolume(bool)));
	}
	else{
	    unremovable_pvs_present = TRUE;
	    pv_used_layout->addWidget(new QLabel(member_pvs[x]->getDeviceName()));
	}
    }

    if(	!pv_check_boxes.size() )                             // no unused pvs present
	pv_unused_layout->addWidget( new QLabel("none") );
    
    if(unremovable_pvs_present){
	layout->addWidget( new QLabel("<b>The following may not be removed</b>") );
	layout->addWidget(pv_used_box);
    }

    enableButtonOk(FALSE);
}

QStringList VGReduceDialog::arguments()
{
    QStringList args;
    int check_box_count = pv_check_boxes.size();

    args << "/sbin/vgreduce"
	 << vg_name;

    for(int x = 0; x < check_box_count; x++){
	if(pv_check_boxes[x]->isChecked())
	    args << pv_check_boxes[x]->getUnmungedText();
    }
    
    return args;
}

void VGReduceDialog::excludeOneVolume(bool)
{
    int boxes_checked = 0;
    int check_box_count = pv_check_boxes.size();
    
    bool selection_made = FALSE;   // True if a least one pv is checked (selected)
    
    for(int x = 0; x < check_box_count; x++)
	if(pv_check_boxes[x]->isChecked()){
	    selection_made = TRUE;
	    boxes_checked++;
	}
    
    enableButtonOk(selection_made); // enable the OK button only if at least one pv is selected

    if( ((boxes_checked + 1) == check_box_count ) && (!unremovable_pvs_present) )
	for(int x = 0; x < check_box_count; x++){
	    if( !(pv_check_boxes[x]->isChecked()) )
		pv_check_boxes[x]->setEnabled(FALSE);
	}
    else
	for(int x = 0; x < check_box_count; x++)
	    pv_check_boxes[x]->setEnabled(TRUE);
    
}

