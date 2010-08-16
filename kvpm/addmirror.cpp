/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include <KIntSpinBox>
#include <KLocale>
#include <KComboBox>

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
        ProcessProgress add_mirror(dialog.arguments(), i18n("Changing Mirror..."), true);
        return true;
    }

    return false;
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
    m_tab_widget->addTab(physical_tab, i18n("Logging and layout") );
    m_general_layout  = new QVBoxLayout;
    m_physical_layout = new QVBoxLayout();
    general_tab->setLayout(m_general_layout);
    physical_tab->setLayout(m_physical_layout);
    
    setupGeneralTab();
    setupPhysicalTab();

    comparePvsNeededPvsAvailable(true);

    connect(m_pv_box, SIGNAL(toggled(bool)), 
	    this, SLOT(clearCheckedPvs(bool)));

    connect(m_pv_box, SIGNAL(toggled(bool)), 
	    this, SLOT(comparePvsNeededPvsAvailable(bool)));

    connect(m_add_mirrors_spin, SIGNAL(valueChanged(int)),
	    this, SLOT(comparePvsNeededPvsAvailable(int)));
}

void AddMirrorDialog::setupGeneralTab()
{
    QLabel *current_mirrors_label = new QLabel();

    m_general_layout->addStretch();
    m_add_mirror_box = new QGroupBox( i18n("Add mirror legs") );
    QVBoxLayout *add_mirror_box_layout = new QVBoxLayout;
    m_add_mirror_box->setLayout(add_mirror_box_layout);
    m_general_layout->addWidget(m_add_mirror_box);

// The number of stripes is really the number of mirror legs 
// for a mirror volume.

    if( m_lv->isMirror() ){
        m_current_leg_count = m_lv->getSegmentStripes(0);
        current_mirrors_label->setText( i18n("Existing mirror legs: %1", m_current_leg_count) );
        m_add_mirror_box->setCheckable( true );
    }
    else{
        current_mirrors_label->setText( i18n("Existing mirror legs: none") );
        m_current_leg_count = 1;   // Even without a mirror it is using a pv.
        m_add_mirror_box->setCheckable( false );
    }

    add_mirror_box_layout->addWidget(current_mirrors_label);
    QHBoxLayout *spin_box_layout = new QHBoxLayout();
    
    QLabel *add_mirrors_label = new QLabel( i18n("Add mirror legs: ") );
    m_add_mirrors_spin = new KIntSpinBox(1, 10, 1, 1, this);
    spin_box_layout->addWidget(add_mirrors_label);
    spin_box_layout->addWidget(m_add_mirrors_spin);
    add_mirror_box_layout->addLayout(spin_box_layout);
    add_mirror_box_layout->addStretch();
  
    QGroupBox *alloc_box = new QGroupBox( i18n("Allocation policy") );
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
    return;
}

void AddMirrorDialog::setupPhysicalTab()
{
    NoMungeCheck *temp_check;
    long long     temp_size;
    QHBoxLayout  *temp_layout;

    m_pv_box = new QGroupBox( i18n("Manually select physical volumes") );
    m_pv_box->setCheckable(true);
    m_pv_box->setChecked(false);
  
    QGroupBox *mirror_leg_group = new QGroupBox( i18n("Suitable for new mirrors") );
  
    QVBoxLayout *mirror_layout     = new QVBoxLayout();
    QVBoxLayout *mirror_leg_layout = new QVBoxLayout();

    m_pv_box->setLayout(mirror_layout);
    mirror_leg_group->setLayout(mirror_leg_layout);
    
    m_physical_layout->addWidget(m_pv_box);
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

    m_stripe_box = new QGroupBox( i18n("Disk striping") );
    QVBoxLayout *striped_layout = new QVBoxLayout();
    m_stripe_box->setCheckable(true);
    m_stripe_box->setChecked(false);
    m_stripe_box->setLayout(striped_layout);
    
    m_stripe_size_combo = new KComboBox();    
    for(int n = 2; (pow(2, n) * 1024) <= m_lv->getVolumeGroup()->getExtentSize() ; n++){
	m_stripe_size_combo->addItem(QString("%1").arg(pow(2, n)) + " KiB");
	m_stripe_size_combo->setItemData(n - 2, QVariant( (int) pow(2, n) ), Qt::UserRole );
    }
    
    QLabel *stripe_size = new QLabel( i18n("Stripe Size: ") );
    m_stripes_number_spin = new KIntSpinBox();
    m_stripes_number_spin->setMinimum(2);
    m_stripes_number_spin->setMaximum(m_lv->getVolumeGroup()->getPhysVolCount());
    QHBoxLayout *stripe_size_layout = new QHBoxLayout();
    stripe_size_layout->addWidget(stripe_size);
    stripe_size_layout->addWidget(m_stripe_size_combo);
    QLabel *stripes_number = new QLabel( i18n("Number of stripes: ") );
    QHBoxLayout *stripes_number_layout = new QHBoxLayout();
    stripes_number_layout->addWidget(stripes_number); 
    stripes_number_layout->addWidget(m_stripes_number_spin);
    striped_layout->addLayout(stripe_size_layout);  
    striped_layout->addLayout(stripes_number_layout);
    m_physical_layout->addWidget(m_stripe_box);

    QGroupBox *log_box = new QGroupBox( i18n("Mirror logging") );
    QVBoxLayout *log_box_layout = new QVBoxLayout;
    m_core_log_button = new QRadioButton( i18n("Memory based log") );
    m_disk_log_button = new QRadioButton( i18n("Disk based log") );
    m_mirrored_log_button = new QRadioButton( i18n("Mirrored disk based log") );
    
    if(m_lv->isMirror()){
        if(m_lv->getLogCount() == 0)
            m_core_log_button->setChecked(true);
        else if(m_lv->getLogCount() == 1)
            m_disk_log_button->setChecked(true);
        else
            m_mirrored_log_button->setChecked(true);
    }
    else
        m_disk_log_button->setChecked(true);
    log_box_layout->addWidget(m_core_log_button);
    log_box_layout->addWidget(m_disk_log_button);
    log_box_layout->addWidget(m_mirrored_log_button);
    log_box->setLayout(log_box_layout);
    m_physical_layout->addWidget(log_box);
    m_physical_layout->addStretch();

    connect(m_disk_log_button, SIGNAL(toggled(bool)),
            this, SLOT(comparePvsNeededPvsAvailable(bool)));

    connect(m_stripes_number_spin, SIGNAL(valueChanged(int)), 
            this, SLOT(comparePvsNeededPvsAvailable(int)));

    connect(m_add_mirrors_spin, SIGNAL(valueChanged(int)), 
            this, SLOT(comparePvsNeededPvsAvailable(int)));

    connect(m_stripe_box, SIGNAL(toggled(bool)), 
            this, SLOT(comparePvsNeededPvsAvailable(bool)));

    connect(m_add_mirror_box, SIGNAL(toggled(bool)), 
            this, SLOT(comparePvsNeededPvsAvailable(bool)));

    comparePvsNeededPvsAvailable();
    return;
}

/* The next function returns a list of physical volumes in 
   use by the mirror as legs or logs. */

QStringList AddMirrorDialog::getPvsInUse()
{
    QList<LogVol *>  mirror_legs = (m_lv->getVolumeGroup())->getLogicalVolumes();
    QStringList pvs_in_use;
    
    if( m_lv->isMirror() ){
	for(int x = mirror_legs.size() - 1; x >= 0; x--){

	    if(  mirror_legs[x]->getOrigin() != m_lv->getName() || 
                 (!mirror_legs[x]->isMirrorLeg() && !mirror_legs[x]->isMirrorLog()) )
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

/* Here we create a string based on all
   the options that the user chose in the
   dialog and feed that to "lvconvert" */

QStringList AddMirrorDialog::arguments()
{
    QStringList args;
    QStringList physical_volumes; // the physical volumes to use for the new mirrors and log
    QString mirror_count_arg;

    if( ! m_add_mirror_box->isCheckable() )
        mirror_count_arg = QString("+%1").arg( m_add_mirrors_spin->value() );
    else if( m_add_mirror_box->isChecked() )
        mirror_count_arg = QString("+%1").arg( m_add_mirrors_spin->value() );
    else
        mirror_count_arg = QString("+0");
    
    for(int x = 0; x < m_pv_leg_checks.size(); x++){
	if( m_pv_leg_checks[x]->isChecked() && m_pv_leg_checks[x]->isEnabled() ){
	    physical_volumes << m_pv_leg_checks[x]->getUnmungedText();
	}
    }

    args << "lvconvert";
    
    if(m_core_log_button->isChecked())
	args << "--mirrorlog" << "core";
    else if(m_mirrored_log_button->isChecked())
        args << "--mirrorlog" << "mirrored";
    else
        args << "--mirrorlog" << "disk";

    if( m_stripe_box->isChecked() ){
        args << "--stripes" <<  QString("%1").arg( m_stripes_number_spin->value());
        args << "--stripesize" << (m_stripe_size_combo->currentText()).remove("KiB").trimmed();
    }

    if ( !inherited_button->isChecked() ){        // "inherited" is what we get if
        args << "--alloc";                        // we don't pass "--alloc" at all
        if ( contiguous_button->isChecked() )     // passing "--alloc" "inherited"
            args << "contiguous" ;                // doesn't work
        else if ( anywhere_button->isChecked() )
            args << "anywhere" ;
        else if ( cling_button->isChecked() )
            args << "cling" ;
        else
            args << "normal" ;
    }

    args << "--mirrors" 
	 << mirror_count_arg
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

    int leg_pvs_checked_count = 0;      // number of pvs selected

    QList <long long> available_pv_bytes;  
    QList <long long> stripe_pv_bytes;  
    int new_stripe_count = 1;
    int total_stripes = 0;   //  stripes per mirror * added mirrors
    int new_log_count;

    if( m_stripe_box->isChecked() )
        new_stripe_count = m_stripes_number_spin->value();

    if( m_add_mirror_box->isChecked() || !m_add_mirror_box->isCheckable() )
        total_stripes = m_add_mirrors_spin->value() * new_stripe_count;

    for(int x = 0; x < m_pv_leg_size.size(); x++){
        if( m_pv_leg_checks[x]->isChecked() || !m_pv_box->isChecked() ){
            leg_pvs_checked_count++;
            available_pv_bytes.append( m_pv_leg_size[x] );
        }
    }

    for(int x = 0; x < total_stripes; x++)
        stripe_pv_bytes.append(0);

    if( m_disk_log_button->isChecked() )
        new_log_count = 1;
    else if(m_mirrored_log_button->isChecked() )
        new_log_count = 2;
    else
        new_log_count = 0;

    qSort(available_pv_bytes);

    for(int x = m_lv->getLogCount(); x < new_log_count; x++){
        if( available_pv_bytes.size() )
            available_pv_bytes.removeFirst();
        else{
            enableButtonOk(false);
            return;
        }
    }

    if(total_stripes){
        while( available_pv_bytes.size() ){
            qSort(available_pv_bytes);
            qSort(stripe_pv_bytes);
            stripe_pv_bytes[0] += available_pv_bytes.takeLast();
        } 
        qSort(stripe_pv_bytes);

        if( stripe_pv_bytes[0] >= ( m_lv->getSize() / new_stripe_count ) )
            enableButtonOk(true);
        else
            enableButtonOk(false);
        return;
    }
    else{
        enableButtonOk(true);
        return;
    }

    enableButtonOk(false);
    return;
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