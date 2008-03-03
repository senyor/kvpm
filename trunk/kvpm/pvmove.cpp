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


#include <KMessageBox>
#include <QtGui>
#include "logvol.h"
#include "masterlist.h"
#include "processprogress.h"
#include "physvol.h"
#include "pvmove.h"
#include "sizetostring.h"
#include "volgroup.h"

extern MasterList *master_list;


bool move_pv(PhysVol *PhysicalVolume)
{
    PVMoveDialog dialog(PhysicalVolume);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress move( dialog.arguments(), "Moving extents...", false );
	return TRUE;
    }
    else
	return FALSE;
}

bool move_pv(LogVol *LogicalVolume)
{
    PVMoveDialog dialog(LogicalVolume);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress move( dialog.arguments(), "Moving extents...", false );
	return TRUE;
    }
    else
	return FALSE;
}

PVMoveDialog::PVMoveDialog(PhysVol *PhysicalVolume, QWidget *parent) : KDialog(parent) 
{
    PhysVol *pv = PhysicalVolume;
    QList<PhysVol *> volgroup_physvols;

    vg = master_list->getVolGroupByName(pv->getVolumeGroupName());
    
    move_lv = FALSE;
    source_pvs << pv->getDeviceName();
    
    volgroup_physvols = vg->getPhysicalVolumes();

    for(int x = volgroup_physvols.size() - 1 ; x >= 0; x--)
	if(volgroup_physvols[x]->getDeviceName() == pv->getDeviceName())
	    volgroup_physvols.removeAt(x);

    needed_space_total = pv->getUsed();
    for(int x = 0; x < volgroup_physvols.size(); x++){
	destination_pvs << volgroup_physvols[x]->getDeviceName();
	free_space << volgroup_physvols[x]->getUnused();
    }
    buildDialog();
}

PVMoveDialog::PVMoveDialog(LogVol *LogicalVolume, QWidget *parent) : KDialog(parent)
{
    QList<PhysVol *> volgroup_physvols;
    
    move_lv = TRUE;
    lv = LogicalVolume;
    vg = lv->getVolumeGroup();

    volgroup_physvols = vg->getPhysicalVolumes();
    for(int x = 0; x < lv->getSegmentCount(); x++)
	source_pvs << lv->getDevicePath(x);

/* If there is only on physical volume in the group then
   a pv move will hve no place to go */

    if(volgroup_physvols.size() < 2){
	KMessageBox::error(this, tr("Only one physical volume is "
				    "assigned to this volume group. "
				    "At least two are required to move extents:\n"
				    "One source and one or more destination"), 
			   "Insufficient physical volumes");
	QEventLoop loop(this);
	loop.exec();
	reject();
    }
    
    for(int x = 0; x < volgroup_physvols.size(); x++){
	destination_pvs << volgroup_physvols[x]->getDeviceName();
	free_space << volgroup_physvols[x]->getUnused();
    }


/* if there is only one source physical volumes possible on this logical volume
   then we eliminate it from the possible destinations pv list completely. */

    if(source_pvs.size() == 1){
	for(int x = (destination_pvs.size() - 1); x >= 0; x--){
	    if( destination_pvs[x] == source_pvs[0] )
		destination_pvs.removeAt(x);
	}
    }

    buildDialog();
}

void PVMoveDialog::buildDialog()
{
    QRadioButton *radio_button;
    NoMungeCheck *check_box;

    setWindowTitle(tr("Move Physical Volume Extents"));
    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout;
    dialog_body->setLayout(layout);

    if(move_lv){
	layout->addWidget( new QLabel("<b>Move only physical extents on:</b>") );
	layout->addWidget( new QLabel("<b>" + lv->getFullName() + "</b>") );
    }

    QGroupBox *radio_group = new QGroupBox("Source Physical Volume");
    QVBoxLayout *radio_group_layout = new QVBoxLayout();
    radio_group->setLayout(radio_group_layout);
    layout->addWidget(radio_group);

    int source_pv_count = source_pvs.size();
    if( source_pv_count > 1){
	for(int x = 0; x < source_pv_count; x++){
	    radio_button = new QRadioButton(source_pvs[x]);
	    if( !x )
		radio_button->setChecked(TRUE);
	    radio_group_layout->addWidget(radio_button);
	    radio_buttons.append(radio_button);

	    connect(radio_button, SIGNAL(toggled(bool)), 
		    this, SLOT(disableDestination(bool)));
	}
    }
    else
	radio_group_layout->addWidget( new QLabel(source_pvs[0]) );

    QGroupBox *check_group = new QGroupBox("Destination Physical Volumes");
    QVBoxLayout *check_layout = new QVBoxLayout();
    check_group->setLayout(check_layout);
    layout->addWidget(check_group);
    check_box_any = new QCheckBox("Any available volume");
    check_layout->addWidget(check_box_any);
    check_layout->addSpacing(5);

    QHBoxLayout *check_layout_lower = new QHBoxLayout();
    check_layout->addLayout(check_layout_lower);

    QVBoxLayout *check_layout_left  = new QVBoxLayout();
    QVBoxLayout *check_layout_right = new QVBoxLayout();
    check_layout_lower->addLayout(check_layout_left);
    check_layout_lower->addLayout(check_layout_right);

    int destination_pv_count = destination_pvs.size();
    for (int x = 0; x < destination_pv_count; x++){

	check_box = new NoMungeCheck(destination_pvs[x]);
	check_box->setEnabled(FALSE);
	check_layout_left->addWidget(check_box);
	check_layout_right->addWidget(new QLabel("Free space: " + sizeToString(free_space[x])));
	free_space_total += free_space[x];
	check_boxes.append(check_box);

	connect(check_box, SIGNAL(toggled(bool)), 
		this, SLOT(calculateSpace(bool)));
    }

    check_layout->addSpacing(5);
    free_space_total_label = new QLabel();
    needed_space_total_label = new QLabel();
    check_layout->addWidget(free_space_total_label);
    check_layout->addWidget(needed_space_total_label);

    connect(check_box_any, SIGNAL(toggled(bool)), 
	    this, SLOT(checkBoxEnable(bool)));

    check_box_any->setChecked(TRUE);
}

void PVMoveDialog::calculateSpace(bool)
{
    free_space_total = 0;

    for(int x = 0; x < check_boxes.size(); x++)
	if(check_boxes[x]->isChecked())
	    free_space_total += free_space[x];

    free_space_total_label->setText("Selected space total: " + sizeToString(free_space_total));

    if(move_lv){
	if(radio_buttons.size() > 1){
	    for(int x = 0; x < radio_buttons.size(); x++){
		if(radio_buttons[x]->isChecked())
		    needed_space_total = lv->getSpaceOnPhysicalVolume(source_pvs[x]);
	    }
	}
	else
	    needed_space_total = lv->getSpaceOnPhysicalVolume(source_pvs[0]);
    }

    needed_space_total_label->setText("Required space total: " + sizeToString(needed_space_total));
    
    if(free_space_total < needed_space_total)
	enableButtonOk(FALSE);
    else
	enableButtonOk(TRUE);
}

void PVMoveDialog::checkBoxEnable(bool checked)
{
    if(checked)
	for(int x = 0; x < check_boxes.size(); x++){
	    check_boxes[x]->setChecked(TRUE);
	    check_boxes[x]->setEnabled(FALSE);
	}
    else
	for(int x = 0; x < check_boxes.size(); x++)
	    check_boxes[x]->setEnabled(TRUE);
    disableDestination(TRUE);
}

void PVMoveDialog::disableDestination(bool)
{
    QString source_pv;
    
    for(int x = 0; x < radio_buttons.size(); x++)
	if(radio_buttons[x]->isChecked())
	    source_pv = source_pvs[x];

    if(check_box_any->isChecked())
	for(int x = 0; x < check_boxes.size(); x++){
	    check_boxes[x]->setEnabled(FALSE);
	    check_boxes[x]->setChecked(TRUE);
	    if(source_pv == destination_pvs[x])
		check_boxes[x]->setChecked(FALSE);
	}
    else
	for(int x = 0; x < check_boxes.size(); x++){
	    check_boxes[x]->setEnabled(TRUE);
	    if(source_pv == destination_pvs[x]){
		check_boxes[x]->setChecked(FALSE);
		check_boxes[x]->setEnabled(FALSE);
	    }
	}
    calculateSpace(TRUE);
}

QStringList PVMoveDialog::arguments()
{
    QStringList args;
    QString source;
    
    args << "pvmove" << "--background";

    if(move_lv){
	args << "--name";
	args << lv->getFullName();
    }
    if(source_pvs.size() > 1)
	for(int x = 0; x < radio_buttons.size(); x++){
	    if(radio_buttons[x]->isChecked())
		source = source_pvs[x];
	}
    else
	source = source_pvs[0];

    args << source;

    if(!check_box_any->isChecked())
	for(int x = 0; x < check_boxes.size(); x++)
	    if(check_boxes[x]->isChecked())
		args << check_boxes[x]->getUnmungedText();

    return args;
}
