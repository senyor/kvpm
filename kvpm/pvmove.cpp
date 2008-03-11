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


bool move_pv(PhysVol *physicalVolume)
{
    PVMoveDialog dialog(physicalVolume);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress move( dialog.arguments(), "Moving extents...", false );
	return true;
    }
    else
	return false;
}

bool move_pv(LogVol *logicalVolume)
{
    PVMoveDialog dialog(logicalVolume);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress move( dialog.arguments(), "Moving extents...", false );
	return true;
    }
    else
	return false;
}

bool restart_pvmove()
{

    QStringList args;
    QString message;

    message.append("Do you wish to restart all interupted physical volume moves?");

    if(KMessageBox::questionYesNo( 0, message) == 3){      // 3 = "yes" button

        args << "pvmove";

        ProcessProgress resize(args, "Restarting pvmove...");
        return true;
    }
    else
        return false;
}

bool stop_pvmove()
{

    QStringList args;
    QString message;

    message.append("Do you wish to abort all physical volume moves");
    message.append(" currently in progress?");
    
    if(KMessageBox::questionYesNo( 0, message) == 3){      // 3 = "yes" button

        args << "pvmove" << "--abort";

        ProcessProgress resize(args, "Stopping pvmove...");
        return true;
    }
    else
        return false;
}


PVMoveDialog::PVMoveDialog(PhysVol *physicalVolume, QWidget *parent) : KDialog(parent) 
{

    PhysVol  *pv = physicalVolume;
    VolGroup *vg = master_list->getVolGroupByName(pv->getVolumeGroupName());
    m_destination_pvs = vg->getPhysicalVolumes();
    
    move_lv = false;
    m_source_pvs.append(pv);

    for(int x = m_destination_pvs.size() - 1 ; x >= 0; x--)
	if(m_destination_pvs[x]->getDeviceName() == m_source_pvs[0]->getDeviceName())
	    m_destination_pvs.removeAt(x);

    buildDialog();
}

PVMoveDialog::PVMoveDialog(LogVol *logicalVolume, QWidget *parent) : 
    KDialog(parent), 
    m_lv(logicalVolume)
{
    QStringList physical_volume_paths;
    
    move_lv = true;
    m_destination_pvs = (m_lv->getVolumeGroup())->getPhysicalVolumes();
	
    physical_volume_paths << m_lv->getDevicePathAll();

    for(int x = 0; x < physical_volume_paths.size(); x++)
	m_source_pvs.append(master_list->getPhysVolByName( physical_volume_paths[x] ));
    

    
/* If there is only on physical volume in the group then
   a pv move will hve no place to go */

    if(m_destination_pvs.size() < 2){
	KMessageBox::error(this, tr("Only one physical volume is "
				    "assigned to this volume group. "
				    "At least two are required to move extents:\n"
				    "One source and one or more destination"), 
			            "Insufficient physical volumes");
	QEventLoop loop(this);
	loop.exec();
	reject();
    }
    

/* if there is only one source physical volumes possible on this logical volume
   then we eliminate it from the possible destinations pv list completely. */

    if(m_source_pvs.size() == 1){
	for(int x = (m_destination_pvs.size() - 1); x >= 0; x--){
	    if( m_destination_pvs[x] == m_source_pvs[0] )
		m_destination_pvs.removeAt(x);
	}
    }

    buildDialog();
}

void PVMoveDialog::buildDialog()
{
    QLabel *label;
    QRadioButton *radio_button;
    NoMungeCheck *check_box;
    long long pv_free_space;
    long long pv_used_space;
    
    setWindowTitle(tr("Move Physical Volume Extents"));
    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout;
    dialog_body->setLayout(layout);

    if(move_lv){
	layout->addWidget( new QLabel("<b>Move only physical extents on:</b>") );
	layout->addWidget( new QLabel("<b>" + m_lv->getFullName() + "</b>") );
    }

    QGroupBox *radio_group = new QGroupBox("Source Physical Volume");
    QHBoxLayout *radio_group_layout = new QHBoxLayout();
    radio_group->setLayout(radio_group_layout);
    layout->addWidget(radio_group);
    QVBoxLayout *radio_button_layout = new QVBoxLayout();
    QVBoxLayout *radio_label_layout  = new QVBoxLayout();
    radio_group_layout->addLayout(radio_button_layout);
    radio_group_layout->addLayout(radio_label_layout);
    
    if( m_source_pvs.size() > 1){
	for(int x = 0; x < m_source_pvs.size(); x++){
	    radio_button = new QRadioButton( m_source_pvs[x]->getDeviceName() );

	    if( !x )
		radio_button->setChecked(true);

	    radio_button_layout->addWidget(radio_button);
	    radio_buttons.append(radio_button);
	    pv_used_space = m_lv->getSpaceOnPhysicalVolume(m_source_pvs[x]->getDeviceName());
	    label = new QLabel("Used space: " + sizeToString( pv_used_space ) );
	    radio_label_layout->addWidget(label);
	    
	    connect(radio_button, SIGNAL(toggled(bool)), 
		    this, SLOT(disableDestination(bool)));
	}
    }
    else{
	radio_button_layout->addWidget( new QLabel( m_source_pvs[0]->getDeviceName() ) );
	pv_used_space = m_lv->getSpaceOnPhysicalVolume( m_source_pvs[0]->getDeviceName() );
	label = new QLabel("Used space: " + sizeToString( pv_used_space ) );
	radio_label_layout->addWidget(label);
    }
    
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

    int destination_pv_count = m_destination_pvs.size();
    for (int x = 0; x < destination_pv_count; x++){

	check_box = new NoMungeCheck( m_destination_pvs[x]->getDeviceName() );
	check_box->setEnabled(false);
	check_layout_left->addWidget(check_box);
	pv_free_space = m_destination_pvs[x]->getUnused();
	check_layout_right->addWidget(new QLabel("Free space: " + 
						 sizeToString( pv_free_space )));

	check_boxes.append(check_box);

	connect(check_box, SIGNAL(toggled(bool)), 
		this, SLOT(calculateSpace(bool)));
    }

    check_layout->addSpacing(5);
    free_space_total_label = new QLabel();
    check_layout->addWidget(free_space_total_label);

    connect(check_box_any, SIGNAL(toggled(bool)), 
	    this, SLOT(checkBoxEnable(bool)));

    check_box_any->setChecked(true);
}

void PVMoveDialog::calculateSpace(bool)
{
    long long free_space_total = 0;
    long long needed_space_total = 0;
    QString device_name;
    
    for(int x = 0; x < check_boxes.size(); x++){
	if(check_boxes[x]->isChecked())
	    free_space_total += m_destination_pvs[x]->getUnused();
    }
    
    free_space_total_label->setText("Selected space total: " + 
				    sizeToString(free_space_total));

    if(move_lv){
	if(radio_buttons.size() > 1){
	    for(int x = 0; x < radio_buttons.size(); x++){
		if(radio_buttons[x]->isChecked()){
		    device_name = m_source_pvs[x]->getDeviceName();
		    needed_space_total = m_lv->getSpaceOnPhysicalVolume( device_name );
		}
	    }
	}
	else{
	    device_name = m_source_pvs[0]->getDeviceName();
	    needed_space_total = m_lv->getSpaceOnPhysicalVolume( device_name );
	}
    }
	    
    if(free_space_total < needed_space_total)
	enableButtonOk(false);
    else
	enableButtonOk(true);
}

void PVMoveDialog::checkBoxEnable(bool checked)
{
    if(checked){
	for(int x = 0; x < check_boxes.size(); x++){
	    check_boxes[x]->setChecked(true);
	    check_boxes[x]->setEnabled(false);
	}
    }
    else{
	for(int x = 0; x < check_boxes.size(); x++)
	    check_boxes[x]->setEnabled(true);
    }
    
    disableDestination(true);
}

void PVMoveDialog::disableDestination(bool)
{
    QString source_pv;
    
    for(int x = 0; x < radio_buttons.size(); x++){
	if(radio_buttons[x]->isChecked())
	    source_pv = m_source_pvs[x]->getDeviceName();
    }

    if(check_box_any->isChecked()){
	for(int x = 0; x < check_boxes.size(); x++){
	    check_boxes[x]->setEnabled(false);
	    check_boxes[x]->setChecked(true);
	    if( source_pv == m_destination_pvs[x]->getDeviceName() )
		check_boxes[x]->setChecked(false);
	}
    }
    else{
	for(int x = 0; x < check_boxes.size(); x++){
	    check_boxes[x]->setEnabled(true);
	    if( source_pv == m_destination_pvs[x]->getDeviceName() ){
		check_boxes[x]->setChecked(false);
		check_boxes[x]->setEnabled(false);
	    }
	}
    }
    
    calculateSpace(true);
}

QStringList PVMoveDialog::arguments()
{
    QStringList args;
    QString source;
    
    args << "pvmove" << "--background";

    if(move_lv){
	args << "--name";
	args << m_lv->getFullName();
    }

    if(m_source_pvs.size() > 1){
	for(int x = 0; x < radio_buttons.size(); x++){
	    if(radio_buttons[x]->isChecked())
		source = m_source_pvs[x]->getDeviceName();
	}
    }
    else{
	source = m_source_pvs[0]->getDeviceName();
    }
    
    args << source;

    if(!check_box_any->isChecked()){
	for(int x = 0; x < check_boxes.size(); x++){
	    if(check_boxes[x]->isChecked())
		args << check_boxes[x]->getUnmungedText();
	}
    }
    
    return args;
}
