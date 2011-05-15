/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <KLocale>
#include <QtGui>

#include "logvol.h"
#include "misc.h"
#include "processprogress.h"
#include "removemirror.h"
#include "volgroup.h"

bool remove_mirror(LogVol *logicalVolume)
{
    RemoveMirrorDialog dialog(logicalVolume);
    dialog.exec();

    if(dialog.result() == QDialog::Accepted){
        ProcessProgress remove_mirror(dialog.arguments(), i18n("Removing mirror...") );
        return true;
    }
    else{
        return false;
    }
}

RemoveMirrorDialog::RemoveMirrorDialog(LogVol *logicalVolume, QWidget *parent):
    KDialog(parent),
    m_lv(logicalVolume)
{
    NoMungeCheck *temp_check;
    QStringList   pv_names;
    
    m_vg = m_lv->getVolumeGroup();
    QList<LogVol *> lvs = m_vg->getLogicalVolumes();

    setWindowTitle( i18n("Remove mirrors") );

    QWidget *dialog_body = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;
    dialog_body->setLayout(layout);
    setMainWidget(dialog_body);
    
    QLabel *message = new QLabel( i18n("Select the mirror legs to remove:") );
    layout->addWidget(message);

    for(int x = lvs.size() - 1; x >= 0 ;x--){
	if( lvs[x]->isMirrorLeg() && (lvs[x]->getOrigin() == m_lv->getName()) ){

	    temp_check = new NoMungeCheck( lvs[x]->getName() );
	    pv_names = lvs[x]->getDevicePathAll();  // mirror legs have only one pv
	    temp_check->setAlternateText( pv_names[0] );
	    m_mirror_leg_checks.append(temp_check);
	    layout->addWidget(temp_check);
	    
	    connect(temp_check, SIGNAL(stateChanged(int)),
		    this ,SLOT(validateCheckStates(int)));
	}
    }
}

/* Here we create a string based on all
   the options that the user chose in the
   dialog and feed that to "lvconvert"     
*/

QStringList RemoveMirrorDialog::arguments()
{
    int mirror_count = m_mirror_leg_checks.size();
    QStringList args;
    QStringList legs;       // mirror legs (actually pv names) being deleted
    
    for(int x = 0; x < m_mirror_leg_checks.size(); x++){
	if( m_mirror_leg_checks[x]->isChecked() ){
	    legs << m_mirror_leg_checks[x]->getAlternateText();
	    mirror_count--;
	}
    }

    args << "lvconvert"
	 << "--mirrors" 
	 << QString("%1").arg(mirror_count - 1)
	 << m_lv->getFullName()
	 << legs;
    
    return args;
}

/* One leg of the mirror must always be left intact, 
   so we make certain at least one check box is left
   unchecked. The unchecked one is disabled. */

void RemoveMirrorDialog::validateCheckStates(int)
{

    int check_box_count = m_mirror_leg_checks.size();
    int checked_count = 0;
    
    for(int x = 0; x < check_box_count; x++){
	if( m_mirror_leg_checks[x]->isChecked() ){
	    checked_count++;
	}
    }

    if( checked_count == (check_box_count - 1) ){

	for(int x = 0; x < check_box_count; x++){
	    if( !m_mirror_leg_checks[x]->isChecked() ){
		m_mirror_leg_checks[x]->setEnabled(false);
	    }
	}
    }
    else{
	for(int x = 0; x < check_box_count; x++){
	    m_mirror_leg_checks[x]->setEnabled(true);

	}

    }
    
}