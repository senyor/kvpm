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

#include "masterlist.h"
#include "nomungecheck.h"
#include "processprogress.h"
#include "vgreduce.h"
#include "volgroup.h"

extern MasterList *master_list;

bool reduce_vg(VolGroup *volumeGroup)
{
    VGReduceDialog dialog(volumeGroup);
    dialog.exec();

    if(dialog.result() == QDialog::Accepted){
        ProcessProgress reduce( dialog.arguments(), "Reducing vg...", true );
        return true;
    }

    return false;
}

VGReduceDialog::VGReduceDialog(VolGroup *volumeGroup, QWidget *parent) : KDialog(parent)
{
    setWindowTitle(tr("Reduce Volume Group"));
    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout;
    dialog_body->setLayout(layout);

    m_unremovable_pvs_present = false;
    m_vg_name = volumeGroup->getName();

    QLabel *label = new QLabel( QString( "Select any of the following physical volumes" ) +
				" to remove them from volume group <b>" +
				m_vg_name +
				"</b>" );
    label->setWordWrap(true);
    layout->addWidget(label);

    QGroupBox *pv_unused_box = new QGroupBox("Unused physical volumes");
    QVBoxLayout *pv_unused_layout = new QVBoxLayout;
    pv_unused_box->setLayout(pv_unused_layout);
    layout->addWidget(pv_unused_box);

    QGroupBox *pv_used_box = new QGroupBox("In use physical volumes");
    QVBoxLayout *pv_used_layout = new QVBoxLayout;
    pv_used_box->setLayout(pv_used_layout);

    QList<PhysVol *> member_pvs = volumeGroup->getPhysicalVolumes();
    int pv_count = volumeGroup->getPhysVolCount(); 
    NoMungeCheck *pv_check = NULL;
    
    for(int x = 0; x < pv_count; x++){
	if( !( member_pvs[x]->getUsed() ) ){                  // only unused pvs can be removed
	    pv_check = new NoMungeCheck( member_pvs[x]->getDeviceName() );
	    m_pv_check_boxes.append(pv_check);
	    pv_unused_layout->addWidget(pv_check);
	    connect(pv_check, SIGNAL(toggled(bool)), this, SLOT(excludeOneVolume(bool)));
	}
	else{
	    m_unremovable_pvs_present = true;
	    pv_used_layout->addWidget(new QLabel(member_pvs[x]->getDeviceName()));
	}
    }

    if(	!m_pv_check_boxes.size() )                             // no unused pvs present
	pv_unused_layout->addWidget( new QLabel("none") );
    
    if(m_unremovable_pvs_present){
	layout->addWidget( new QLabel("<b>The following may not be removed</b>") );
	layout->addWidget(pv_used_box);
    }

    enableButtonOk(false);
}

QStringList VGReduceDialog::arguments()
{
    QStringList args;
    int check_box_count = m_pv_check_boxes.size();

    args << "vgreduce"
	 << m_vg_name;

    for(int x = 0; x < check_box_count; x++){
	if(m_pv_check_boxes[x]->isChecked())
	    args << m_pv_check_boxes[x]->getUnmungedText();
    }
    
    return args;
}

void VGReduceDialog::excludeOneVolume(bool)
{
    int boxes_checked = 0;
    int check_box_count = m_pv_check_boxes.size();
    
    bool selection_made = false;   // True if a least one pv is checked (selected)
    
    for(int x = 0; x < check_box_count; x++){
	if(m_pv_check_boxes[x]->isChecked()){
	    selection_made = true;
	    boxes_checked++;
	}
    }
    
    enableButtonOk(selection_made); // enable the OK button only if at least one pv is selected

    if( ((boxes_checked + 1) == check_box_count ) && (!m_unremovable_pvs_present) ){
	
	for(int x = 0; x < check_box_count; x++){
	    if( !(m_pv_check_boxes[x]->isChecked()) )
		m_pv_check_boxes[x]->setEnabled(false);
	}
    }
    else{
	for(int x = 0; x < check_box_count; x++)
	    m_pv_check_boxes[x]->setEnabled(true);
    }
}

