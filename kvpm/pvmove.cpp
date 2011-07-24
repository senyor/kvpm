/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include <KPushButton>
#include <KMessageBox>
#include <KLocale>
#include <QtGui>

#include "logvol.h"
#include "masterlist.h"
#include "processprogress.h"
#include "physvol.h"
#include "pvcheckbox.h"
#include "pvmove.h"
#include "misc.h"
#include "volgroup.h"

extern MasterList *master_list;


bool move_pv(PhysVol *physicalVolume)
{
    PVMoveDialog dialog(physicalVolume);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress move( dialog.arguments(), i18n("Moving extents..."), false );
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
        ProcessProgress move( dialog.arguments(), i18n("Moving extents..."), false );
	return true;
    }
    else
	return false;
}

bool restart_pvmove()
{
    QStringList args;
    QString message = i18n("Do you wish to restart all interupted physical volume moves?");

    if(KMessageBox::questionYesNo( 0, message) == 3){      // 3 = "yes" button

        args << "pvmove";

        ProcessProgress resize(args, i18n("Restarting pvmove...") );
        return true;
    }
    else
        return false;
}

bool stop_pvmove()
{

    QStringList args;
    QString message = i18n("Do you wish to abort all physical volume moves " 
			   "currently in progress?");
    
    if(KMessageBox::questionYesNo( 0, message) == 3){      // 3 = "yes" button

        args << "pvmove" << "--abort";

        ProcessProgress resize(args, i18n("Stopping pvmove...") );
        return true;
    }
    else
        return false;
}


PVMoveDialog::PVMoveDialog(PhysVol *physicalVolume, QWidget *parent) : KDialog(parent) 
{

    PhysVol  *pv = physicalVolume;
    VolGroup *vg = pv->getVolGroup();
    m_target_pvs = vg->getPhysicalVolumes();
    
    move_lv = false;
    m_source_pvs.append(pv);

    for(int x = m_target_pvs.size() - 1 ; x >= 0; x--)
	if(m_target_pvs[x]->getName() == m_source_pvs[0]->getName())
	    m_target_pvs.removeAt(x);

    removeEmptyTargets();
    buildDialog();
}

PVMoveDialog::PVMoveDialog(LogVol *logicalVolume, QWidget *parent) : 
    KDialog(parent), 
    m_lv(logicalVolume)
{
    QStringList physical_volume_paths;

    QList<LogVol *>  lv_list;  // these 3 are only for mirrors
    LogVol *leg; 
    int lv_count;

    move_lv = true;
    m_target_pvs = m_lv->getVolumeGroup()->getPhysicalVolumes();

    // Turns out mirrors don't work with pv move so the following isn't needed yet.

    if( m_lv->isMirror() ){  // find the mirror legs and get the pvs

        lv_list = m_lv->getVolumeGroup()->getLogicalVolumes();
        lv_count = lv_list.size();

        for(int x = 0; x < lv_count; x++){

            leg = lv_list[x];

            if( ( leg->getOrigin() == m_lv->getName() ) && ( leg->isMirrorLog() ||
                                                             leg->isMirrorLeg() ||
                                                             leg->isVirtual()   ||
                                                             leg->isMirror() ) )  {
                
             
                physical_volume_paths << leg->getDevicePathAll();

            }

            if( physical_volume_paths.size() > 1 ){

                physical_volume_paths.sort();

                for( int x = ( physical_volume_paths.size() - 1 ); x > 0 ;x--){
                    if( physical_volume_paths[x] == physical_volume_paths[x - 1] )
                        physical_volume_paths.removeAt(x);
                } 
            }
        }

        for(int x = 0; x < physical_volume_paths.size(); x++)
            m_source_pvs.append( m_lv->getVolumeGroup()->getPhysVolByName( physical_volume_paths[x] ));

    }
    else{	
        physical_volume_paths << m_lv->getDevicePathAll();
        
        for(int x = 0; x < physical_volume_paths.size(); x++)
            m_source_pvs.append(m_lv->getVolumeGroup()->getPhysVolByName( physical_volume_paths[x] ));
    }

/* if there is only one source physical volumes possible on this logical volume
   then we eliminate it from the possible target pv list completely. */

    if(m_source_pvs.size() == 1){
	for(int x = (m_target_pvs.size() - 1); x >= 0; x--){
	    if( m_target_pvs[x] == m_source_pvs[0] )
		m_target_pvs.removeAt(x);
	}
    }

    removeEmptyTargets();
    buildDialog();
}

void PVMoveDialog::removeEmptyTargets(){

    for(int x = (m_target_pvs.size() - 1); x >= 0; x--){
        if( m_target_pvs[x]->getUnused() <= 0 )
            m_target_pvs.removeAt(x);
    }

/* If there is only one physical volume in the group or they are 
   all full then a pv move will have no place to go */

    if(m_target_pvs.size() < 1){
	KMessageBox::error(this, i18n("There are no available physical volumes with space to move too"));
	QEventLoop loop(this);
	loop.exec();
	reject();
    }
}

void PVMoveDialog::buildDialog()
{
    QLabel *label;
    NoMungeRadioButton *radio_button;
    
    setWindowTitle( i18n("Move Physical Volume Extents") );
    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout;
    dialog_body->setLayout(layout);

    if(move_lv){
        layout->addWidget( new QLabel( i18n("<b>Move only physical extents on:</b>") ) );
	layout->addWidget( new QLabel("<b>" + m_lv->getFullName() + "</b>") );
    }

    QGroupBox *radio_group = new QGroupBox( i18n("Source Physical Volumes") );
    QGridLayout *radio_layout = new QGridLayout();
    radio_group->setLayout(radio_layout);
    layout->addWidget(radio_group);
    QHBoxLayout *lower_layout = new QHBoxLayout;
    layout->addLayout(lower_layout);

    m_pv_checkbox = new PVCheckBox(m_target_pvs);
    lower_layout->addWidget(m_pv_checkbox);

    int radio_count = m_source_pvs.size();
    if( radio_count > 1){
	for(int x = 0; x < m_source_pvs.size(); x++){

	    if(move_lv)
	        m_pv_used_space = m_lv->getSpaceOnPhysicalVolume(m_source_pvs[x]->getName());
	    else
	        m_pv_used_space = m_source_pvs[x]->getSize() - m_source_pvs[x]->getUnused();

	    radio_button = new NoMungeRadioButton( m_source_pvs[x]->getName() + "  " + sizeToString(m_pv_used_space));
            radio_button->setAlternateText( m_source_pvs[x]->getName() );

            if(radio_count < 11 )
                radio_layout->addWidget(radio_button, x % 5, x / 5);
            else if (radio_count % 3 == 0)
                radio_layout->addWidget(radio_button, x % (radio_count / 3), x / (radio_count / 3));
            else
                radio_layout->addWidget(radio_button, x % ( (radio_count + 2) / 3), x / ( (radio_count + 2) / 3));

	    m_radio_buttons.append(radio_button);
	    
	    connect(radio_button, SIGNAL(toggled(bool)), 
		    this, SLOT(disableDestination()));

	    if( !x )
		radio_button->setChecked(true);
	}
    }
    else{

 	radio_layout->addWidget( new QLabel( m_source_pvs[0]->getName() ) );

	if(move_lv)
	    m_pv_used_space = m_lv->getSpaceOnPhysicalVolume(m_source_pvs[0]->getName());
	else
	    m_pv_used_space = m_source_pvs[0]->getSize() - m_source_pvs[0]->getUnused();
    
	label = new QLabel("Required space: " + sizeToString( m_pv_used_space ) );
	radio_layout->addWidget(label);
    }

    QGroupBox *alloc_box = new QGroupBox( i18n("Allocation Policy") );
    QVBoxLayout *alloc_box_layout = new QVBoxLayout;
    m_normal_button     = new QRadioButton( i18n("Normal") );
    m_contiguous_button = new QRadioButton( i18n("Contiguous") );
    m_anywhere_button   = new QRadioButton( i18n("Anywhere") );
    m_inherited_button  = new QRadioButton( i18n("Inherited") );
    m_inherited_button->setChecked(true);
    m_cling_button      = new QRadioButton( i18n("Cling") );
    alloc_box_layout->addWidget(m_normal_button);
    alloc_box_layout->addWidget(m_contiguous_button);
    alloc_box_layout->addWidget(m_anywhere_button);
    alloc_box_layout->addWidget(m_inherited_button);
    alloc_box_layout->addWidget(m_cling_button);
    alloc_box_layout->addStretch();
    alloc_box->setLayout(alloc_box_layout);
    lower_layout->addWidget(alloc_box);

    connect(m_pv_checkbox, SIGNAL(stateChanged()), 
            this, SLOT(resetOkButton()));

    resetOkButton();
}

void PVMoveDialog::resetOkButton()
{
    long long free_space_total = m_pv_checkbox->getUnusedSpace();
    long long needed_space_total = 0;
    QString device_name;

    if(move_lv){
	if(m_radio_buttons.size() > 1){
	    for(int x = 0; x < m_radio_buttons.size(); x++){
		if(m_radio_buttons[x]->isChecked()){
		    device_name = m_source_pvs[x]->getName();
		    needed_space_total = m_lv->getSpaceOnPhysicalVolume( device_name );
		}
	    }
	}
	else{
	    device_name = m_source_pvs[0]->getName();
	    needed_space_total = m_lv->getSpaceOnPhysicalVolume( device_name );
	}
    }
    else
        needed_space_total = m_pv_used_space; 
	    
    if(free_space_total < needed_space_total)
	enableButtonOk(false);
    else
	enableButtonOk(true);
}

void PVMoveDialog::disableDestination()
{
    PhysVol *source_pv = NULL;
    
    for(int x = 0; x < m_radio_buttons.size(); x++){
	if(m_radio_buttons[x]->isChecked())
	    source_pv = m_source_pvs[x];
    }

    m_pv_checkbox->disableOrigin(source_pv);

    resetOkButton();
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

    if ( !m_inherited_button->isChecked() ){        // "inherited" is what we get if 
        args << "--alloc";                          // we don't pass "--alloc" at all
        if ( m_contiguous_button->isChecked() )     // passing "--alloc" "inherited"  
            args << "contiguous" ;                  // doesn't work                        
        else if ( m_anywhere_button->isChecked() )
            args << "anywhere" ;
        else if ( m_cling_button->isChecked() )
            args << "cling" ;
        else
            args << "normal" ;
    }

    if(m_source_pvs.size() > 1){
	for(int x = 0; x < m_radio_buttons.size(); x++){
	    if(m_radio_buttons[x]->isChecked())
		source = m_source_pvs[x]->getName();
	}
    }
    else{
	source = m_source_pvs[0]->getName();
    }
    
    args << source;

    args << m_pv_checkbox->getNames();

    return args;
}
