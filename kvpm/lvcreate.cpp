/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include "lvcreate.h"

#include <KMessageBox>
#include <KLocale>

#include <QtGui>

#include "fsextend.h"
#include "logvol.h"
#include "misc.h"
#include "mountentry.h"
#include "physvol.h"
#include "pvcheckbox.h"
#include "processprogress.h"
#include "sizeselectorbox.h"
#include "volgroup.h"



/* This class handles both the creation and extension of logical
   volumes and snapshots since both processes are so similar. */

LVCreateDialog::LVCreateDialog(VolGroup *volumeGroup, QWidget *parent):
    KDialog(parent),
    m_vg(volumeGroup)
{
    m_lv = NULL;
    m_extend = false;
    m_snapshot = false;
    m_bailout  = hasInitialErrors();
    m_fs_can_extend = false;

    if ( !m_bailout ){
        setCaption( i18n("Create Logical Volume") );

        m_tab_widget = new KTabWidget(this);
        m_physical_tab = createPhysicalTab();
        m_advanced_tab = createAdvancedTab();
        m_general_tab  = createGeneralTab();
        m_tab_widget->addTab(m_general_tab,  i18nc("The standard common options", "General") );
        m_tab_widget->addTab(m_physical_tab, i18n("Physical layout") );
        m_tab_widget->addTab(m_advanced_tab, i18nc("Less used, dangerous or complex options", "Advanced options") );
        
        setMainWidget(m_tab_widget);
        makeConnections();
        setMaxSize();
    }
}

LVCreateDialog::LVCreateDialog(LogVol *logicalVolume, bool snapshot, QWidget *parent):
    KDialog(parent),
    m_snapshot(snapshot),
    m_lv(logicalVolume)
{
    m_extend = !m_snapshot;
    m_vg = m_lv->getVg();
    m_bailout  = hasInitialErrors();

    if ( !m_bailout ){

        if(m_snapshot)
            setCaption( i18n("Create snapshot Volume") );
        else
            setCaption( i18n("Extend Logical Volume") );
        
        m_tab_widget = new KTabWidget(this);
        m_physical_tab = createPhysicalTab();  // this order is important
        m_advanced_tab = createAdvancedTab();
        m_general_tab  = createGeneralTab();
        m_tab_widget->addTab(m_general_tab,  i18nc("The standard common options", "General") );
        m_tab_widget->addTab(m_physical_tab, i18n("Physical layout") );
        m_tab_widget->addTab(m_advanced_tab, i18nc("Less used, dangerous or complex options", "Advanced options") );
        
        setMainWidget(m_tab_widget);
        makeConnections();
        setMaxSize();
    }
}

void LVCreateDialog::makeConnections()
{
    connect(this, SIGNAL(okClicked()), 
            this, SLOT(commitChanges()));

    connect(m_pv_checkbox, SIGNAL(stateChanged()), 
            this, SLOT(setMaxSize()));

    connect(m_stripe_count_spin, SIGNAL(valueChanged(int)), 
	    this, SLOT(setMaxSize()));

    connect(m_mirror_count_spin, SIGNAL(valueChanged(int)), 
	    this, SLOT(setMaxSize()));

    connect(m_stripe_box, SIGNAL(toggled(bool)), 
	    this, SLOT(setMaxSize()));

    connect(m_mirror_box, SIGNAL(toggled(bool)), 
	    this, SLOT(setMaxSize()));

    connect(m_mirror_box, SIGNAL(toggled(bool)), 
	    this, SLOT(enableMonitoring(bool)));

    connect(m_mirrored_log, SIGNAL(toggled(bool)), 
	    this, SLOT(setMaxSize()));

    connect(m_disk_log, SIGNAL(toggled(bool)), 
	    this, SLOT(setMaxSize()));

    connect(m_core_log, SIGNAL(toggled(bool)), 
	    this, SLOT(setMaxSize()));

    connect(m_size_selector, SIGNAL(stateChanged()), 
            this, SLOT(setMaxSize()));
}

QWidget* LVCreateDialog::createGeneralTab()
{
     m_tag_edit = NULL;

     QWidget *general_tab = new QWidget(this);
     QHBoxLayout *general_layout = new QHBoxLayout;
     general_tab->setLayout(general_layout);
     QGroupBox *volume_box = new QGroupBox(); 
     QVBoxLayout *layout = new QVBoxLayout;
     volume_box->setLayout(layout);
     general_layout->addStretch();
     general_layout->addWidget(volume_box);
     general_layout->addStretch();
     QVBoxLayout *upper_layout = new QVBoxLayout();
     QHBoxLayout *lower_layout = new QHBoxLayout();
     layout->addStretch();
     layout->addLayout(upper_layout);
     layout->addStretch();
     layout->addLayout(lower_layout);

     m_name_is_valid = true;

     if(m_extend){
         volume_box->setTitle( i18n("Extending volume: %1", m_lv->getName()) );
         m_extend_by_label = new QLabel();
         layout->insertWidget( 1, m_extend_by_label );
         m_current_size_label = new QLabel( i18n("Current size: %1", sizeToString( m_lv->getSize() ) )  );
         layout->insertWidget( 2, m_current_size_label );
     }
     else{

         if(m_snapshot)
             volume_box->setTitle( i18n("Creating snapshot of: %1", m_lv->getName()) );
         else
             volume_box->setTitle( i18n("Create new logical volume") );

	 QHBoxLayout *name_layout = new QHBoxLayout();
	 m_name_edit = new KLineEdit();

	 QRegExp rx("[0-9a-zA-Z_\\.][-0-9a-zA-Z_\\.]*");
	 m_name_validator = new QRegExpValidator( rx, m_name_edit );
	 m_name_edit->setValidator(m_name_validator);

	 name_layout->addWidget( new QLabel( i18n("Volume name: ") ) );
	 name_layout->addWidget(m_name_edit);
	 upper_layout->insertLayout(0, name_layout);

	 QHBoxLayout *tag_layout = new QHBoxLayout();
	 m_tag_edit = new KLineEdit();

	 QRegExp rx2("[0-9a-zA-Z_\\.+-]*");
	 m_tag_validator = new QRegExpValidator( rx2, m_tag_edit );
	 m_tag_edit->setValidator(m_tag_validator);

	 tag_layout->addWidget( new QLabel( i18n("Optional tag: ") ) );
	 tag_layout->addWidget(m_tag_edit);
	 upper_layout->insertLayout(1, tag_layout);

	 connect(m_name_edit, SIGNAL(textEdited(QString)), 
		 this, SLOT(validateVolumeName(QString)));

     }

     if(m_extend){
         if( getStripeCount() > 1 ){
             m_stripe_box->setChecked(false);
             m_size_selector = new SizeSelectorBox(m_vg->getExtentSize(), m_lv->getExtents(), 
                                                   getLargestVolume() / m_vg->getExtentSize(), 
                                                   m_lv->getExtents(), true, false);
             m_stripe_box->setChecked(true);
         }
         else{
             m_size_selector = new SizeSelectorBox(m_vg->getExtentSize(), m_lv->getExtents(), 
                                                   getLargestVolume() / m_vg->getExtentSize(), 
                                                   m_lv->getExtents(), true, false);
         }
     }
     else{
         m_size_selector = new SizeSelectorBox(m_vg->getExtentSize(), 0, 
                                               getLargestVolume() / m_vg->getExtentSize() , 
                                               0, true, false);
     }

     upper_layout->addWidget(m_size_selector);

     QGroupBox *volume_limit_box = new QGroupBox( i18n("Maximum volume size") );
     QVBoxLayout *volume_limit_layout = new QVBoxLayout();
     volume_limit_box->setLayout(volume_limit_layout);
     m_max_size_label = new QLabel();
     volume_limit_layout->addWidget(m_max_size_label);
     m_max_extents_label = new QLabel();
     volume_limit_layout->addWidget(m_max_extents_label);
     m_stripe_count_label = new QLabel();
     volume_limit_layout->addWidget(m_stripe_count_label);
     volume_limit_layout->addStretch();
     lower_layout->addWidget(volume_limit_box);

     QGroupBox *alloc_box = new QGroupBox( i18n("Allocation Policy") );
     QVBoxLayout *alloc_box_layout = new QVBoxLayout;
     normal_button     = new QRadioButton( i18nc("The usual way", "Normal") );
     contiguous_button = new QRadioButton( i18n("Contiguous") );
     anywhere_button   = new QRadioButton( i18n("Anywhere") );
     inherited_button  = new QRadioButton( i18nc("Inherited from the parent group", "Inherited") );
     cling_button      = new QRadioButton( i18n("Cling") );

     QString policy = "Inherited";
     if( m_extend )
         policy = m_lv->getPolicy();

     if( policy == "Normal" )
         normal_button->setChecked(true);
     else if( policy == "Contiguous" )
         contiguous_button->setChecked(true);
     else if( policy == "Anywhere" )
         anywhere_button->setChecked(true);
     else if( policy == "Cling" )
         cling_button->setChecked(true);
     else
         inherited_button->setChecked(true);

     alloc_box_layout->addWidget(normal_button);
     alloc_box_layout->addWidget(contiguous_button);
     alloc_box_layout->addWidget(anywhere_button);
     alloc_box_layout->addWidget(inherited_button);
     alloc_box_layout->addWidget(cling_button);
     alloc_box->setLayout(alloc_box_layout);
     lower_layout->addWidget(alloc_box);

     return general_tab;
}

QWidget* LVCreateDialog::createPhysicalTab()
{
    QString message;
    int stripe_index;
    QList<PhysVol *> physical_volumes;

    QVBoxLayout *layout = new QVBoxLayout;
    m_physical_tab = new QWidget(this);
    m_physical_tab->setLayout(layout);
    
    physical_volumes = m_vg->getPhysicalVolumes();
    for(int x = physical_volumes.size() - 1; x >= 0; x--){
        if( physical_volumes[x]->getRemaining() < 1 )  // remove pvs with no free space
            physical_volumes.removeAt(x);
    }

    m_pv_checkbox = new PVCheckBox(physical_volumes);
    layout->addWidget(m_pv_checkbox);

    QVBoxLayout *striped_layout = new QVBoxLayout;
    QHBoxLayout *stripe_size_layout = new QHBoxLayout;
    QHBoxLayout *stripes_number_layout = new QHBoxLayout;
    
    m_stripe_box = new QGroupBox( i18n("Disk striping") );
    m_stripe_box->setCheckable(true);
    m_stripe_box->setChecked(false);
    m_stripe_box->setLayout(striped_layout);

    stripe_size_combo = new KComboBox();    
    m_stripe_box->setEnabled(false);
    for(int n = 2; (pow(2, n) * 1024) <= m_vg->getExtentSize() ; n++){
	stripe_size_combo->addItem(QString("%1").arg(pow(2, n)) + " KiB");
	stripe_size_combo->setItemData(n - 2, QVariant( (int) pow(2, n) ), Qt::UserRole );
        m_stripe_box->setEnabled(true);   // only enabled if the combo box has at least one entry!
    }
    if(physical_volumes.size() < 2)
        m_stripe_box->setEnabled(false);
    
    QLabel *stripe_size = new QLabel( i18n("Stripe Size: ") );
    m_stripe_count_spin = new QSpinBox();
    m_stripe_count_spin->setMinimum(2);
    m_stripe_count_spin->setMaximum(m_vg->getPVCount());
    stripe_size_layout->addWidget(stripe_size);
    stripe_size_layout->addWidget(stripe_size_combo);
    QLabel *stripes_number = new QLabel( i18n("Number of stripes: ") );
    stripes_number_layout->addWidget(stripes_number); 
    stripes_number_layout->addWidget(m_stripe_count_spin);
    striped_layout->addLayout(stripe_size_layout);  
    striped_layout->addLayout(stripes_number_layout);

/* If we are extending a volume we try to match the striping
   of the last segment of that volume, if it was striped */

    if(m_extend){
	int seg_count = 1;
	int seg_stripe_count = 1;
	int seg_stripe_size = 4;
        QList<LogVol *>  logvols;

        if( m_lv->isMirror() ){                        // Tries to match striping to last segment of first leg
            logvols = m_lv->getAllChildrenFlat();
            for(int x = 0; x < logvols.size(); x++){
                if(logvols[x]->isMirrorLeg() && !(logvols[x]->isMirrorLog()) ){
                    seg_count = logvols[x]->getSegmentCount();
                    seg_stripe_count = logvols[x]->getSegmentStripes(seg_count - 1);
                    seg_stripe_size = logvols[x]->getSegmentStripeSize(seg_count - 1);
                    break;
                }
            }
        }
        else{
            seg_count = m_lv->getSegmentCount();
            seg_stripe_count = m_lv->getSegmentStripes(seg_count - 1);
            seg_stripe_size = m_lv->getSegmentStripeSize(seg_count - 1);
	}

	if( seg_stripe_count > 1 && m_stripe_box->isEnabled() ){
	    m_stripe_count_spin->setValue(seg_stripe_count);
            
            if( physical_volumes.size() >= (seg_stripe_count * m_lv->getMirrorCount()) )
                m_stripe_box->setChecked(true);
            else
                m_stripe_box->setChecked(false);

	    stripe_index = stripe_size_combo->findData( QVariant(seg_stripe_size / 1024) );
	    if(stripe_index == -1)
		stripe_index = 0;
	    stripe_size_combo->setCurrentIndex(stripe_index);
	}
    }

    QVBoxLayout *mirror_layout = new QVBoxLayout;
    m_mirror_box = new QGroupBox( i18n("Disk mirrors") );
    m_mirror_box->setCheckable(true);
    m_mirror_box->setChecked(false);
    if(physical_volumes.size() < 2)
        m_mirror_box->setEnabled(false);
    m_mirror_box->setLayout(mirror_layout);

    m_mirrored_log = new QRadioButton( i18n("Mirrored disk based log") );
    m_disk_log = new QRadioButton( i18n("Disk based log") );
    m_core_log = new QRadioButton( i18n("Memory based log") );
    m_disk_log->setChecked(true);
    mirror_layout->addWidget(m_mirrored_log);
    mirror_layout->addWidget(m_disk_log);
    mirror_layout->addWidget(m_core_log);

    QHBoxLayout *mirrors_spin_layout = new QHBoxLayout();
    
    m_mirror_count_spin = new QSpinBox();
    m_mirror_count_spin->setMinimum(2);
    m_mirror_count_spin->setMaximum(m_vg->getPVCount());
    QLabel *mirrors_number_label  = new QLabel( i18n("Number of mirror legs: ") );
    mirrors_spin_layout->addWidget(mirrors_number_label);
    mirrors_spin_layout->addWidget(m_mirror_count_spin);
    mirror_layout->addLayout(mirrors_spin_layout);

    QHBoxLayout *lower_h_layout = new QHBoxLayout;
    QVBoxLayout *lower_v_layout = new QVBoxLayout;
    layout->addLayout(lower_h_layout);
    lower_h_layout->addStretch();
    lower_h_layout->addLayout(lower_v_layout);
    lower_h_layout->addStretch();

    lower_v_layout->addWidget(m_stripe_box);

    if( (!m_extend) && (!m_snapshot) )
	lower_v_layout->addWidget(m_mirror_box);
    else
	m_mirror_box->setEnabled(false);

    return m_physical_tab;
}

QWidget* LVCreateDialog::createAdvancedTab()
{
    QHBoxLayout *advanced_layout = new QHBoxLayout;
    m_advanced_tab = new QWidget(this);
    m_advanced_tab->setLayout(advanced_layout);
    QGroupBox *advanced_box = new QGroupBox();
    QVBoxLayout *layout = new QVBoxLayout;
    advanced_box->setLayout(layout);
    advanced_layout->addStretch();
    advanced_layout->addWidget(advanced_box);
    advanced_layout->addStretch();
    
    m_persistent_box = new QGroupBox( i18n("Device numbering") );
    m_persistent_box->setCheckable(true);
    m_persistent_box->setChecked(false);
    
    m_readonly_check = new QCheckBox();
    m_readonly_check->setText( i18n("Set read only") );
    layout->addWidget(m_readonly_check);
    m_zero_check = new QCheckBox();
    m_zero_check->setText( i18n("Write zeros at volume start") );
    layout->addWidget(m_zero_check);

    if( !m_snapshot && !m_extend ){
    
	connect(m_zero_check, SIGNAL(stateChanged(int)), 
		this ,SLOT(zeroReadonlyCheck(int)));
	connect(m_readonly_check, SIGNAL(stateChanged(int)), 
		this ,SLOT(zeroReadonlyCheck(int)));

        m_zero_check->setChecked(true);
	m_readonly_check->setChecked(false);

    }
    else{
	m_zero_check->setChecked(false);
	m_zero_check->setEnabled(false);
	m_readonly_check->setChecked(false);
	m_readonly_check->setEnabled(false);
    }

    m_monitor_check = new QCheckBox( i18n("Monitor with dmeventd") );
    m_skip_sync_check = new QCheckBox( i18n("Skip initial synchronization of mirror") );

    if(m_snapshot){
        m_monitor_check->setChecked(true);
        m_monitor_check->setEnabled(true);
        m_skip_sync_check->setChecked(false);
        m_skip_sync_check->setEnabled(false);
	layout->addWidget(m_monitor_check);
        layout->addWidget(m_skip_sync_check);
    }
    else if(m_extend){
        m_monitor_check->setChecked(false);
        m_monitor_check->setEnabled(false);
        m_skip_sync_check->setChecked(false);
        m_skip_sync_check->setEnabled(false);
    }
    else{
        m_monitor_check->setChecked(false);
        m_monitor_check->setEnabled(false);
        m_skip_sync_check->setChecked(false);
        m_skip_sync_check->setEnabled(false);
	layout->addWidget(m_monitor_check);
        layout->addWidget(m_skip_sync_check);
    }

    QVBoxLayout *persistent_layout   = new QVBoxLayout;
    QHBoxLayout *minor_number_layout = new QHBoxLayout;
    QHBoxLayout *major_number_layout = new QHBoxLayout;
    m_minor_number_edit = new KLineEdit();
    m_major_number_edit = new KLineEdit();
    QLabel *minor_number = new QLabel( i18n("Device minor number: ") );
    QLabel *major_number = new QLabel( i18n("Device major number: ") );
    major_number_layout->addWidget(major_number);
    major_number_layout->addWidget(m_major_number_edit);
    minor_number_layout->addWidget(minor_number);
    minor_number_layout->addWidget(m_minor_number_edit);
    persistent_layout->addLayout(major_number_layout);
    persistent_layout->addLayout(minor_number_layout);
    QIntValidator *minor_validator = new QIntValidator(m_minor_number_edit); 
    QIntValidator *major_validator = new QIntValidator(m_major_number_edit); 
    minor_validator->setBottom(0);
    major_validator->setBottom(0);
    m_minor_number_edit->setValidator(minor_validator);
    m_major_number_edit->setValidator(major_validator);
    m_persistent_box->setLayout(persistent_layout);
    layout->addWidget(m_persistent_box);
    
    layout->addStretch();

    return m_advanced_tab;
}

void LVCreateDialog::setMaxSize()
{
    const int stripe_count = getStripeCount();
    const int mirror_count = getMirrorCount();
    const long long free_extents  = getLargestVolume() / m_vg->getExtentSize();
    const long long selected_size = m_size_selector->getCurrentSize() * m_vg->getExtentSize(); 

    m_size_selector->setConstrainedMax(free_extents);
    m_max_size_label->setText( i18n("Size: %1", sizeToString( getLargestVolume() )) );
    m_max_extents_label->setText( i18n("Extents: %1", free_extents) );

    if(!m_extend){

        if( m_mirror_box->isChecked() && !m_stripe_box->isChecked() )
            m_stripe_count_label->setText( i18n("(with %1 mirror legs)", mirror_count) );
        else if( !m_mirror_box->isChecked() && m_stripe_box->isChecked() )
            m_stripe_count_label->setText( i18n("(with %1 stripes)", stripe_count) );
        else if( m_mirror_box->isChecked() && m_stripe_box->isChecked()  )
            m_stripe_count_label->setText( i18n("(with %1 mirror legs\nand %2 stripes)", mirror_count, stripe_count) );
        else
            m_stripe_count_label->setText( i18n("(linear volume)") );
    }
    else{
        m_extend_by_label->setText( i18n("Extend by: %1", sizeToString( selected_size - m_lv->getSize() )) );

        if( mirror_count > 1 && !m_stripe_box->isChecked() )
            m_stripe_count_label->setText( i18n("(with %1 mirror legs)", mirror_count) );
        else if( mirror_count < 2 && m_stripe_box->isChecked() )
            m_stripe_count_label->setText( i18n("(with %1 stripes)", stripe_count) );
        else if( mirror_count > 1 && m_stripe_box->isChecked() )
            m_stripe_count_label->setText( i18n("(with %1 mirror legs\nand %2 stripes)", mirror_count, stripe_count) );
        else
            m_stripe_count_label->setText( i18n("(linear volume)") );
    }

    resetOkButton();
}

void LVCreateDialog::validateVolumeName(QString name)
{
    int pos = 0;

    if( m_name_validator->validate(name, pos) == QValidator::Acceptable && name != "." && name != ".." )
	m_name_is_valid = true;
    else if( name.isEmpty() )
	m_name_is_valid = true;
    else
	m_name_is_valid = false;
    
    resetOkButton();
}

void LVCreateDialog::resetOkButton()
{
    const long long max_extents = getLargestVolume() / m_vg->getExtentSize();
    const long long volume_extents = roundExtentsToStripes( m_size_selector->getCurrentSize() );

    if(m_size_selector->isValid()){
        if(!m_extend){
            if( (volume_extents <= max_extents) && (volume_extents > 0) && m_name_is_valid )
                enableButtonOk(true);
            else
                enableButtonOk(false);
        }
        else{
            if( (volume_extents <= max_extents) && (volume_extents > m_lv->getExtents()) && m_name_is_valid )
                enableButtonOk(true);
            else
                enableButtonOk(false);
        }
    }
    else
        enableButtonOk(false);
}

void LVCreateDialog::enableMonitoring(bool checked)
{
    if(checked){
        m_monitor_check->setChecked(true);
        m_monitor_check->setEnabled(true);
        m_skip_sync_check->setChecked(true);
        m_skip_sync_check->setEnabled(true);
    }
    else{
        if(!m_snapshot){
            m_monitor_check->setChecked(false);
            m_monitor_check->setEnabled(false);
        }
        m_skip_sync_check->setChecked(false);
        m_skip_sync_check->setEnabled(false);
    }
}

/* largest volume that can be created given the pvs , striping and mirrors
   selected. This includes the size of the already existing volume if we
   are extending a volume */

long long LVCreateDialog::getLargestVolume() 
{
    QList <long long> available_pv_bytes = m_pv_checkbox->getRemainingSpaceList();  
    QList <long long> stripe_pv_bytes;  
    const int total_stripes = getStripeCount() * getMirrorCount();
    int log_count;

    for(int x = 0; x < total_stripes; x++)
        stripe_pv_bytes.append(0);

    if( m_mirror_box->isChecked() ){
        if( m_disk_log->isChecked() )
            log_count = 1;
        else if(m_mirrored_log->isChecked() )
            log_count = 2;
        else
            log_count = 0;
    }
    else
        log_count = 0;

    qSort(available_pv_bytes);

    for(int x = 0; x < log_count; x++){
        if( available_pv_bytes.size() )
            available_pv_bytes.removeFirst();
        else
            return 0;
    }

    while( available_pv_bytes.size() ){
        qSort(stripe_pv_bytes);
        stripe_pv_bytes[0] += available_pv_bytes.takeLast();
    }
    qSort(stripe_pv_bytes);

    if(!m_extend)
        return stripe_pv_bytes[0] * getStripeCount();
    else
        return ( stripe_pv_bytes[0] * getStripeCount() ) + m_lv->getSize();
}

int LVCreateDialog::getStripeCount()
{
    if(m_stripe_box->isChecked())
	return m_stripe_count_spin->value();
    else
	return 1;
}

int LVCreateDialog::getMirrorCount()
{
    if(m_extend)
        return m_lv->getMirrorCount();
    else if( m_mirror_box->isChecked() )
	return m_mirror_count_spin->value();
    else
	return 1;
}

void LVCreateDialog::zeroReadonlyCheck(int)
{
    if( !m_snapshot && !m_extend){  
	if (m_zero_check->isChecked()){
	    m_readonly_check->setChecked(false);
	    m_readonly_check->setEnabled(false);
	}
	else
	    m_readonly_check->setEnabled(true);
	
	if (m_readonly_check->isChecked()){
	    m_zero_check->setChecked(false);
	    m_zero_check->setEnabled(false);
	}
	else
	    m_zero_check->setEnabled(true);
    }
    else
	m_readonly_check->setEnabled(true);
}

/* Here we create a stringlist of arguments based on all
   the options that the user chose in the dialog. */

QStringList LVCreateDialog::argumentsLV()
{
    QString program_to_run;
    QStringList args;
    QVariant stripe_size;
    const int stripes = getStripeCount();
    const int mirrors = getMirrorCount();
    long long extents = m_size_selector->getCurrentSize();

    if(m_tag_edit){
        if( ! ( m_tag_edit->text() ).isEmpty() )
            args << "--addtag" << m_tag_edit->text();
    } 

    if(m_persistent_box->isChecked()){
	args << "--persistent" << "y";
	args << "--major" << m_major_number_edit->text();
	args << "--minor" << m_minor_number_edit->text();
    }

    stripe_size = stripe_size_combo->itemData(stripe_size_combo->currentIndex(), Qt::UserRole);

    if( m_stripe_box->isChecked() || m_extend){
	args << "--stripes" << QString("%1").arg(stripes);
	args << "--stripesize" << QString("%1").arg( stripe_size.toLongLong() );
    }
    
    if( !m_extend ){
	if ( m_readonly_check->isChecked() )
	    args << "--permission" << "r" ;
	else 
	    args << "--permission" << "rw" ;

	if( !m_snapshot && !m_extend ){
	    if( m_zero_check->isChecked() )
		args << "--zero" << "y";
	    else
		args << "--zero" << "n";

	    if( m_mirror_box->isChecked() ){
		args << "--mirrors" << QString("%1").arg( mirrors - 1 );

		if( m_skip_sync_check->isChecked() )
                    args << "--nosync";

		if( m_mirrored_log->isChecked() )
		    args << "--mirrorlog" << "mirrored";
		else if( m_disk_log->isChecked() )
		    args << "--mirrorlog" << "disk";
		else
		    args << "--mirrorlog" << "core";
	    }
	}
    }

    if( m_monitor_check->isEnabled() ){ 
        args << "--monitor"; 
        if( m_monitor_check->isChecked() )
            args << "y";
        else
            args << "n";
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

    if(m_extend)
        extents -= m_lv->getExtents();

    extents = roundExtentsToStripes(extents);

    args << "--extents" << QString("+%1").arg(extents);

    if( !m_extend && !m_snapshot ){                           // create a standard volume
	program_to_run = "lvcreate";
	
	if( ! (m_name_edit->text()).isEmpty() )
	    args << "--name" << m_name_edit->text();
	
	args << m_vg->getName();
    }
    else if( m_snapshot ){                                  // create a snapshot
	program_to_run = "lvcreate";
	
	args << "--snapshot";
	    
	if( ! (m_name_edit->text()).isEmpty() )
	    args << "--name" << m_name_edit->text() ;
	args << m_lv->getFullName();
    }
    else{                                               // extend the current volume
	program_to_run = "lvextend";
	args <<	m_lv->getFullName();
    }

    args << m_pv_checkbox->getNames();

    args.prepend(program_to_run);
    return args;
}

// make the number of extents divivsible by the stripe X mirror count then round up

long long LVCreateDialog::roundExtentsToStripes(long long extents)
{
    const int stripes = getStripeCount();
    const int mirrors = getMirrorCount();
    const long long max_extents = getLargestVolume() / m_vg->getExtentSize();

    // The next part should only need to reference stripes, not the mirror count
    // but a bug in lvm requires it. Remove this when fixed.

    if( stripes > 1 ){  
        if( extents % ( stripes * mirrors )){  
            extents = extents / ( stripes * mirrors );
            extents = extents * ( stripes * mirrors );
            if(extents + ( stripes * mirrors ) <= max_extents)
                extents += ( stripes * mirrors );
        }
    }

    return extents;
}

// This function checks for problems that would make showing this dialog pointless
// returns true if there are problems and is used to set m_bailout.

bool LVCreateDialog::hasInitialErrors()
{
    if( !m_vg->getFreeExtents() ){
        KMessageBox::error(this, i18n("There is no free space left in the volume group") );
        return true;
    }

    if(m_extend){

        const QString warning_message = i18n("If this volume has a filesystem or data, it will need to be extended <b>separately</b>. "
                                             "Currently, only the ext2, ext3, ext4, xfs, jfs, ntfs and reiserfs file systems are "
                                             "supported for extension. The correct executable for extention must also be present. "); 
        
        if( m_lv->isOrigin() ){
            if( m_lv->isOpen() ){
                KMessageBox::error(this, i18n("Snapshot origins cannot be extended while open or mounted") );
                return true;
            }
            
            const QList<LogVol *> snap_shots = m_lv->getSnapshots();
            
            for(int x = 0; x < snap_shots.size(); x++){
                if( snap_shots[x]->isOpen() ){
                    KMessageBox::error(this, i18n("Volumes cannot be extended with open or mounted snapshots") );
                    return true;
                }
            }
        }

        m_fs_can_extend = fs_can_extend( m_lv->getFilesystem() );

        if( !( m_fs_can_extend || m_lv->isSnap() ) ){
            if(KMessageBox::warningContinueCancel(this, warning_message) != KMessageBox::Continue){
                return true;
            }
        }
    }

    return false;
}

bool LVCreateDialog::bailout()
{
    return m_bailout;
}

void LVCreateDialog::commitChanges()
{
    QStringList lvchange_args;
    hide();

    if( !m_extend ){
        ProcessProgress create_lv( argumentsLV() );
        return;
    }
    else{
        const QString mapper_path = m_lv->getMapperPath();
        const QString fs = m_lv->getFilesystem();

        if( m_lv->isOrigin() ){

            lvchange_args << "lvchange" << "-an" << mapper_path;            
            ProcessProgress deactivate_lv(lvchange_args);
            if( deactivate_lv.exitCode() ){
                KMessageBox::error(0, i18n("Volume deactivation failed, volume not extended") );
                return;
            }
            
            ProcessProgress extend_origin( argumentsLV() );
            if( extend_origin.exitCode() ){
                KMessageBox::error(0, i18n("Volume extention failed") );
                return;
            }
            
            lvchange_args.clear();
            lvchange_args << "lvchange" << "-ay" << mapper_path;
            ProcessProgress activate_lv(lvchange_args);
            if( activate_lv.exitCode() ){
                KMessageBox::error(0, i18n("Volume activation failed, filesystem not extended") );
                return;
            }

            if(m_fs_can_extend)
                fs_extend( m_lv->getMapperPath(), fs, true );		    
            return;
        }
        else{
            ProcessProgress extend_lv( argumentsLV() );
            if( !extend_lv.exitCode() && !m_lv->isSnap() && m_fs_can_extend )
                fs_extend( mapper_path, fs, true );		    
            return;
        }
    }
}

