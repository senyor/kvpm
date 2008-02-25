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

#include <KIntSpinBox>

#include <QtGui>

#include "addmirror.h"
#include "logvol.h"
#include "nomungecheck.h"
#include "physvol.h"
#include "processprogress.h"
#include "sizetostring.h"
#include "volgroup.h"


bool add_mirror(LogVol *logicalVolume)
{
   AddMirrorDialog dialog(logicalVolume);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress add_mirror(dialog.arguments(), "Adding Mirror...", false);
        return true;
    }
    else
	return false;
}


AddMirrorDialog::AddMirrorDialog(LogVol *logicalVolume, QWidget *parent):
    KDialog(parent), m_lv(logicalVolume)
{
    setWindowTitle(tr("Add Mirror"));

    m_logical_volume_name = logicalVolume->getFullName();
 
    m_tab_widget = new KTabWidget();
    setMainWidget(m_tab_widget);
   
    QWidget *general_tab  = new QWidget(this);
    QWidget *physical_tab = new QWidget(this);
    m_tab_widget->addTab(general_tab,  "General");
    m_tab_widget->addTab(physical_tab, "Physical layout");
    m_general_layout  = new QVBoxLayout;
    m_physical_layout = new QVBoxLayout();
    general_tab->setLayout(m_general_layout);
    physical_tab->setLayout(m_physical_layout);
    
    setupGeneralTab();
    setupPhysicalTab();
    
}

void AddMirrorDialog::setupGeneralTab()
{
    QLabel *current_mirrors_label = new QLabel();

// The number of stripes is really the number of mirror legs 
// for a mirror volume and there is only one segment.

    if( m_lv->isMirror() ){
	m_current_leg_count = m_lv->getSegmentStripes(0);
	current_mirrors_label->setText( QString("Existing mirror legs: %1")
					.arg(m_current_leg_count) );
    }
    else{
	current_mirrors_label->setText("Existing mirror legs: none");
	m_current_leg_count = 0;
    }
    
    m_general_layout->addStretch();
    m_general_layout->addWidget(current_mirrors_label);
    QHBoxLayout *spin_box_layout = new QHBoxLayout();

    QLabel *add_mirrors_label = new QLabel("Add mirror legs: ");
    m_add_mirrors_spin = new KIntSpinBox(1, 10, 1, 1, this);
    spin_box_layout->addWidget(add_mirrors_label);
    spin_box_layout->addWidget(m_add_mirrors_spin);
    m_general_layout->addLayout(spin_box_layout);
    m_general_layout->addStretch();
    
    
    QGroupBox *alloc_box = new QGroupBox("Allocation Policy");
    QVBoxLayout *alloc_box_layout = new QVBoxLayout;
    normal_button     = new QRadioButton("Normal");
    contiguous_button = new QRadioButton("Contiguous");
    anywhere_button   = new QRadioButton("Anywhere");
    inherited_button  = new QRadioButton("Inherited");
    cling_button      = new QRadioButton("Cling");
    normal_button->setChecked(true);
    alloc_box_layout->addWidget(normal_button);
    alloc_box_layout->addWidget(contiguous_button);
    alloc_box_layout->addWidget(anywhere_button);
    alloc_box_layout->addWidget(inherited_button);
    alloc_box_layout->addWidget(cling_button);
    alloc_box->setLayout(alloc_box_layout);
    m_general_layout->addWidget(alloc_box);

}

void AddMirrorDialog::setupPhysicalTab()
{
    NoMungeCheck *temp_check;
    
    QGroupBox *log_box = new QGroupBox("Mirror logging");
    QVBoxLayout *log_box_layout = new QVBoxLayout;
    core_log = new QRadioButton("Memory based log");
    disk_log = new QRadioButton("Disk based log");
    disk_log->setChecked(true);
    log_box_layout->addWidget(disk_log);
    log_box_layout->addWidget(core_log);
    log_box->setLayout(log_box_layout);
    m_physical_layout->addWidget(log_box);
    m_physical_layout->addStretch();

    m_mirror_group = new QGroupBox("Manually select physical volumes");
    m_mirror_group->setCheckable(true);
    m_mirror_group->setChecked(false);

    QGroupBox *mirror_leg_group = new QGroupBox("Suitable for mirror legs or log");
    m_mirror_log_group = new QGroupBox("Suitable for mirror log only");

    QVBoxLayout *mirror_layout     = new QVBoxLayout();
    QVBoxLayout *mirror_leg_layout = new QVBoxLayout();
    QVBoxLayout *mirror_log_layout = new QVBoxLayout();
    m_mirror_group->setLayout(mirror_layout);
    mirror_leg_group->setLayout(mirror_leg_layout);
    m_mirror_log_group->setLayout(mirror_log_layout);

    m_physical_layout->addWidget(m_mirror_group);
    m_physical_layout->addStretch();

    mirror_layout->addWidget(mirror_leg_group);
    mirror_layout->addWidget(m_mirror_log_group);

    QList<PhysVol *> leg_physical_volumes = (m_lv->getVolumeGroup())->getPhysicalVolumes();
    QList<PhysVol *> log_physical_volumes;

    QStringList pvs_in_use = getPvsInUse();
    
// pvs with a mirror leg or log already on them aren't
// suitable for another so we remove those here

    for(int x = leg_physical_volumes.size() - 1; x >= 0; x--){
	if( !leg_physical_volumes[x]->isAllocateable() ){
	    leg_physical_volumes.removeAt(x);
	}
	else{
	    for(int y = 0; y < pvs_in_use.size() ; y++){
		if( leg_physical_volumes[x]->getDeviceName() == pvs_in_use[y] ){
		    leg_physical_volumes.removeAt(x);
		    break;
		}
	    }
	}
    }

    log_physical_volumes = leg_physical_volumes;

// if the pv is big enough it can hold a leg otherwise only
// the log will fit, if it has any room at all.

    for(int x = 0; x < leg_physical_volumes.size(); x++){
	if( leg_physical_volumes[x]->getUnused() < m_lv->getSize() ){
	    leg_physical_volumes.removeAt(x);
	    x--;
	}
	else{
	    temp_check = new NoMungeCheck( leg_physical_volumes[x]->getDeviceName() );
	    temp_check->setEnabled(true);
	    temp_check->setChecked(false);
	    m_pv_checks.append(temp_check);
	    mirror_leg_layout->addWidget(temp_check);
	}
    }

    for(int x = 0; x < log_physical_volumes.size(); x++){
	if( (log_physical_volumes[x]->getUnused() >= m_lv->getSize()) ||
	    (log_physical_volumes[x]->getUnused() == 0) )
	{
	    log_physical_volumes.removeAt(x);
	    x--;
	}
	else{
	    temp_check = new NoMungeCheck( log_physical_volumes[x]->getDeviceName() );
	    temp_check->setEnabled(true);
	    temp_check->setChecked(false);
	    m_pv_checks.append(temp_check);
	    mirror_log_layout->addWidget(temp_check);
	}
    }

    connect(disk_log,     SIGNAL(toggled(bool)), 
	    this, SLOT(showMirrorLogBox(bool)));

    connect(m_mirror_group, SIGNAL(toggled(bool)), 
	    this, SLOT(enableMirrorLogBox(bool)));
}

QStringList AddMirrorDialog::getPvsInUse()
{
    QList<LogVol *>  mirror_legs = (m_lv->getVolumeGroup())->getLogicalVolumes();
    QStringList pvs_in_use;
    
    if( m_lv->isMirror() ){
	for(int x = mirror_legs.size() - 1; x >= 0; x--){
	    if(  mirror_legs[x]->getOrigin() != m_lv->getName() ||
		 (!mirror_legs[x]->isMirrorLeg() &&
		  !mirror_legs[x]->isMirrorLog()) )
	    {
		mirror_legs.removeAt(x);
	    }
	    else
		pvs_in_use << mirror_legs[x]->getDevicePathAll();
	}
    }
    else{
	pvs_in_use << m_lv->getDevicePathAll();
	mirror_legs.clear();
    }

    return pvs_in_use;
}

void AddMirrorDialog::showMirrorLogBox(bool show)
{
    if( show ){
	m_mirror_log_group->show();

	if ( m_mirror_group->isChecked() )
	    m_mirror_log_group->setEnabled(true);
	else
	    m_mirror_log_group->setEnabled(false);
    }
    else{
	m_mirror_log_group->hide();
	m_mirror_log_group->setEnabled(false);
    }
}

void AddMirrorDialog::enableMirrorLogBox(bool enable)
{
    if( enable )
	m_mirror_log_group->setEnabled(true);
    else
	m_mirror_log_group->setEnabled(false);
}


/* Here we create a string based on all
   the options that the user chose in the
   dialog and feed that to "lvconvert"     
*/

QStringList AddMirrorDialog::arguments()
{
    QStringList args;
    QStringList physical_volumes; // the physical volumes to use for the new mirrors and log

    for(int x = 0; x < m_pv_checks.size(); x++){
	if( m_pv_checks[x]->isChecked() && m_pv_checks[x]->isEnabled() ){
	    physical_volumes << m_pv_checks[x]->getUnmungedText();
	}
    }

    args << "/sbin/lvconvert";
    
    if(core_log->isChecked())
	args << "--corelog";

    args << "--mirrors" 
	 << QString("%1").arg( m_add_mirrors_spin->value() )
	 << "--background"
	 << m_logical_volume_name
	 << physical_volumes;

    qDebug() << "args: " << args;
    
    return args;
}
