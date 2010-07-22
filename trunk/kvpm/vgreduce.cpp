/*
 *
 * 
 * Copyright (C) 2008, 2010 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include <lvm2app.h>

#include <KMessageBox>
#include <KLocale>
#include <QtGui>

#include "masterlist.h"
#include "nomungecheck.h"
#include "vgreduce.h"
#include "volgroup.h"
#include "physvol.h"

extern MasterList *master_list;

bool reduce_vg(VolGroup *volumeGroup)
{
    VGReduceDialog dialog(volumeGroup);
    dialog.exec();

    if(dialog.result() == QDialog::Accepted)
        return true;
    else
        return false;
}

VGReduceDialog::VGReduceDialog(VolGroup *volumeGroup, QWidget *parent) : KDialog(parent), m_vg(volumeGroup)
{
    setWindowTitle( i18n("Reduce Volume Group") );
    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout;
    dialog_body->setLayout(layout);

    m_unremovable_pvs_present = false;
    QString vg_name = m_vg->getName();

    QLabel *label = new QLabel( i18n( "Select any of the following physical volumes to "
				      "remove them from volume group <b>%1</b>").arg(vg_name));

    label->setWordWrap(true);
    layout->addWidget(label);

    QGroupBox *pv_unused_box = new QGroupBox( i18n("Unused physical volumes") );
    QVBoxLayout *pv_unused_layout = new QVBoxLayout;
    pv_unused_box->setLayout(pv_unused_layout);
    layout->addWidget(pv_unused_box);

    QGroupBox *pv_used_box = new QGroupBox( i18n("In use physical volumes") );
    QVBoxLayout *pv_used_layout = new QVBoxLayout;
    pv_used_box->setLayout(pv_used_layout);

    QList<PhysVol *> member_pvs = m_vg->getPhysicalVolumes();
    int pv_count = m_vg->getPhysVolCount(); 
    NoMungeCheck *pv_check = NULL;
    
    for(int x = 0; x < pv_count; x++){
	if( !( member_pvs[x]->getSize() - member_pvs[x]->getUnused() ) ){ // only unused pvs can be removed
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
        pv_unused_layout->addWidget( new QLabel( i18n("none") ) );
    
    if(m_unremovable_pvs_present){
        layout->addWidget( new QLabel( i18n("<b>The following may not be removed</b>") ) );
	layout->addWidget(pv_used_box);
    }

    connect(this, SIGNAL(okClicked()), 
            this, SLOT(commitChanges()));

    enableButtonOk(false);
}

void VGReduceDialog::commitChanges()
{
    QStringList pvs;
    lvm_t  lvm = NULL;
    vg_t vg_dm = NULL;

    QStringList pv_list; // pvs to remove by name
    int check_box_count = m_pv_check_boxes.size();

    for(int x = 0; x < check_box_count; x++){
	if(m_pv_check_boxes[x]->isChecked())
	    pv_list << m_pv_check_boxes[x]->getUnmungedText();
    }
    
    if( (lvm = lvm_init(NULL)) ){
        if( (vg_dm = lvm_vg_open(lvm, m_vg->getName().toAscii().data(), "w", NULL)) ){
            for(int x = 0; x < pv_list.size(); x++){
                if( lvm_vg_reduce(vg_dm, pv_list[x].toAscii().data()) )
                    KMessageBox::error(0, QString(lvm_errmsg(lvm)));
            }
            if( lvm_vg_write(vg_dm) )
                KMessageBox::error(0, QString(lvm_errmsg(lvm)));
            lvm_vg_close(vg_dm);
        }
        else
            KMessageBox::error(0, QString(lvm_errmsg(lvm)));
        lvm_quit(lvm);
    }
    else
        KMessageBox::error(0, QString(lvm_errmsg(lvm)));

    return;
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

