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
#include <KLocale>

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
    ProcessProgress add_mirror(dialog.arguments(), i18n("Adding Mirror..."), true);
    return true;
  }
  else{
    return false;
  }
}


AddMirrorDialog::AddMirrorDialog(LogVol *logicalVolume, QWidget *parent):
    KDialog(parent), 
    m_lv(logicalVolume)
{
    setWindowTitle( i18n("Add Mirror") );

    m_logical_volume_name = logicalVolume->getFullName();
 
    m_tab_widget = new KTabWidget();
    setMainWidget(m_tab_widget);
   
    QWidget *general_tab  = new QWidget(this);
    QWidget *physical_tab = new QWidget(this);
    m_tab_widget->addTab(general_tab,  i18n("General") );
    m_tab_widget->addTab(physical_tab, i18n("Physical layout") );
    m_general_layout  = new QVBoxLayout;
    m_physical_layout = new QVBoxLayout();
    general_tab->setLayout(m_general_layout);
    physical_tab->setLayout(m_physical_layout);
    
    setupGeneralTab();
    setupPhysicalTab();

    comparePvsNeededPvsAvailable(true);

    connect(m_mirror_group, SIGNAL(toggled(bool)), 
	    this, SLOT(clearCheckedPvs(bool)));

    connect(m_mirror_group, SIGNAL(toggled(bool)), 
	    this, SLOT(comparePvsNeededPvsAvailable(bool)));

    connect(m_add_mirrors_spin, SIGNAL(valueChanged(int)),
	    this, SLOT(comparePvsNeededPvsAvailable(int)));
}

void AddMirrorDialog::setupGeneralTab()
{
  QLabel *current_mirrors_label = new QLabel();

// The number of stripes is really the number of mirror legs 
// for a mirror volume.

  if( m_lv->isMirror() ){
    m_current_leg_count = m_lv->getSegmentStripes(0);
    current_mirrors_label->setText( i18n("Existing mirror legs: %1")
				    .arg(m_current_leg_count) );
  }
  else{
    current_mirrors_label->setText( i18n("Existing mirror legs: none") );
    m_current_leg_count = 1;   // Even without a mirror it is using a pv.
  }
    
  m_general_layout->addStretch();
  m_general_layout->addWidget(current_mirrors_label);
  QHBoxLayout *spin_box_layout = new QHBoxLayout();
  
  QLabel *add_mirrors_label = new QLabel( i18n("Add mirror legs: ") );
  m_add_mirrors_spin = new KIntSpinBox(0, 10, 1, 1, this);
  spin_box_layout->addWidget(add_mirrors_label);
  spin_box_layout->addWidget(m_add_mirrors_spin);
  m_general_layout->addLayout(spin_box_layout);
  m_general_layout->addStretch();
  
  QGroupBox *alloc_box = new QGroupBox( i18n("Allocation Policy") );
  QVBoxLayout *alloc_box_layout = new QVBoxLayout;
  normal_button     = new QRadioButton( i18n("Normal") );
  contiguous_button = new QRadioButton( i18n("Contiguous") );
  anywhere_button   = new QRadioButton( i18n("Anywhere") );
  inherited_button  = new QRadioButton( i18n("Inherited") );
  cling_button      = new QRadioButton( i18n("Cling") );
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
  long long     temp_size;
  QHBoxLayout  *temp_layout;

  QGroupBox *log_box = new QGroupBox( i18n("Mirror logging") );
  QVBoxLayout *log_box_layout = new QVBoxLayout;
  core_log = new QRadioButton( i18n("Memory based log") );
  disk_log = new QRadioButton( i18n("Disk based log") );
  disk_log->setChecked(true);
  log_box_layout->addWidget(disk_log);
  log_box_layout->addWidget(core_log);
  log_box->setLayout(log_box_layout);
  m_physical_layout->addWidget(log_box);
  m_physical_layout->addStretch();

  connect(disk_log, SIGNAL(toggled(bool)),
	  this, SLOT(comparePvsNeededPvsAvailable(bool)));

  m_mirror_group = new QGroupBox( i18n("Manually select physical volumes") );
  m_mirror_group->setCheckable(true);
  m_mirror_group->setChecked(false);
  
  QGroupBox *mirror_leg_group = new QGroupBox( i18n("Suitable for new mirrors") );
  
  QVBoxLayout *mirror_layout     = new QVBoxLayout();
  QVBoxLayout *mirror_leg_layout = new QVBoxLayout();

  m_mirror_group->setLayout(mirror_layout);
  mirror_leg_group->setLayout(mirror_leg_layout);
  
  m_physical_layout->addWidget(m_mirror_group);
  m_physical_layout->addStretch();
  
  mirror_layout->addWidget(mirror_leg_group);
  
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

  for(int x = 0; x < leg_physical_volumes.size(); x++){

    temp_check = new NoMungeCheck( leg_physical_volumes[x]->getDeviceName() );
    temp_check->setEnabled(true);
    temp_check->setChecked(false);
    m_pv_leg_checks.append(temp_check);

    temp_layout = new QHBoxLayout;
    temp_layout->addWidget(temp_check);

    connect(temp_check, SIGNAL(toggled(bool)),
	    this, SLOT(comparePvsNeededPvsAvailable(bool)));

    temp_size = leg_physical_volumes[x]->getUnused();
    m_pv_leg_size.append( temp_size );
    temp_layout->addWidget(new QLabel(sizeToString(temp_size)) );

    mirror_leg_layout->addLayout( temp_layout ); 

  }

  comparePvsNeededPvsAvailable();
}

/* The next function returns a list of physical volumes in 
   use by the mirror as legs or log. It also sets or clears the 
   m_mirror_has_log boolean */

QStringList AddMirrorDialog::getPvsInUse()
{
    QList<LogVol *>  mirror_legs = (m_lv->getVolumeGroup())->getLogicalVolumes();
    QStringList pvs_in_use;
    
    if( m_lv->isMirror() ){
	m_mirror_has_log = false;
	for(int x = mirror_legs.size() - 1; x >= 0; x--){

	    if(  mirror_legs[x]->getOrigin() == m_lv->getName() &&
		 mirror_legs[x]->isMirrorLog() )
	    {
		m_mirror_has_log = true;
	    }

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
	m_mirror_has_log = false;
    }

    return pvs_in_use;
}

/* Here we create a string based on all
   the options that the user chose in the
   dialog and feed that to "lvconvert"     
*/

QStringList AddMirrorDialog::arguments()
{
    QStringList args;
    QStringList physical_volumes; // the physical volumes to use for the new mirrors and log

    for(int x = 0; x < m_pv_leg_checks.size(); x++){
	if( m_pv_leg_checks[x]->isChecked() && m_pv_leg_checks[x]->isEnabled() ){
	    physical_volumes << m_pv_leg_checks[x]->getUnmungedText();
	}
    }

    args << "lvconvert";
    
    if(core_log->isChecked())
	args << "--corelog";

    args << "--mirrors" 
	 << QString("+%1").arg( m_add_mirrors_spin->value() )
	 << "--background"
	 << m_logical_volume_name
	 << physical_volumes;

    return args;
}


/* Enable or disable the OK button based on having
   enough physical volumes either checked, if we are
   selecting them manually, or available if doing it
   automatically. At least one pv for each mirror leg
   and one for the log is needed. We also total up
   the space needed. */

// This function still doesn't guaranty the pvs are
// sufficient unless adding only one mirror leg or  
// adding only a mirror log.    << FIX ME >>

void AddMirrorDialog::comparePvsNeededPvsAvailable(bool)
{
    comparePvsNeededPvsAvailable();
}

void AddMirrorDialog::comparePvsNeededPvsAvailable(int)
{
    comparePvsNeededPvsAvailable();
}

void AddMirrorDialog::comparePvsNeededPvsAvailable()
{

  long long leg_pvs_checked_size = 0; // bytes that are available in
                                      // pvs that the user has checked

  long long leg_pvs_needed_size = 0;  // bytes needed by selected mirrors
  int leg_pvs_checked_count = 0;      // number of pvs selected
  
  leg_pvs_needed_size = m_lv->getSize() * (long long)m_add_mirrors_spin->value();

  for(int x = 0; x < m_pv_leg_size.size(); x++){
    if( m_pv_leg_checks[x]->isChecked() ||
	!m_mirror_group->isChecked() ){
      leg_pvs_checked_count++;
      leg_pvs_checked_size += m_pv_leg_size[x];
    }
  }

  if( !m_mirror_has_log && disk_log->isChecked() ){
    leg_pvs_needed_size += (m_lv->getVolumeGroup())->getExtentSize(); // logs take 1 extent 

    if( leg_pvs_checked_size >= leg_pvs_needed_size &&
	m_add_mirrors_spin->value() + 1 <= leg_pvs_checked_count ){
      enableButtonOk(true);
    }
    else{
      enableButtonOk(false);
    }
  }
  else{
    if( leg_pvs_checked_size >= leg_pvs_needed_size &&
	m_add_mirrors_spin->value() <= leg_pvs_checked_count ){
      enableButtonOk(true);
    }
    else{
      enableButtonOk(false);
    }
  }

}

/* clear the selected physical volumes when the
   mirror group box is unchecked */

void AddMirrorDialog::clearCheckedPvs(bool checked)
{
    if( !checked ){
	for(int x = 0; x < m_pv_leg_checks.size(); x++)
	    m_pv_leg_checks[x]->setChecked(false);

    }
}
