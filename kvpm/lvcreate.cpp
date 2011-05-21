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


#include <sys/mount.h>
#include <errno.h>
#include <cmath>

#include <QtGui>
#include <KMessageBox>
#include <KLocale>

#include "fsextend.h"
#include "logvol.h"
#include "lvcreate.h"
#include "mountentry.h"
#include "physvol.h"
#include "pvcheckbox.h"
#include "processprogress.h"
#include "misc.h"
#include "volgroup.h"


bool lv_create(VolGroup *volumeGroup)
{
    LVCreateDialog dialog(volumeGroup);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress create_lv(dialog.argumentsLV(), i18n("Creating volume..."), true);
	return true;
    }
    else
	return false;
}

bool snapshot_create(LogVol *logicalVolume)
{
    LVCreateDialog dialog(logicalVolume, true);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress create_lv(dialog.argumentsLV(), i18n("Creating snapshot..."), true);
	return true;
    }
    else
	return false;
}

bool lv_extend(LogVol *logicalVolume)
{
    QString fs = logicalVolume->getFilesystem();

    QString warning_message = i18n("Currently only the ext2/3/4, xfs, jfs " 
				   "and reiserfs file systems are "
				   "supported for file system extention. If this logical "
				   "volume has a filesystem or data, it will need to be " 
				   "extended separately!");

    if( fs == "xfs"  || fs == "jfs"  || fs == "reiserfs" || 
        fs == "ext2" || fs == "ext3" || fs == "ext4" || logicalVolume->isSnap() ){

        LVCreateDialog dialog(logicalVolume, false);
        dialog.exec();

        if( dialog.result() == QDialog::Accepted ){
            ProcessProgress extend_lv(dialog.argumentsLV(), i18n("Extending volume..."), true);
            if( !extend_lv.exitCode() && !logicalVolume->isSnap() )
                fs_extend( logicalVolume->getMapperPath(), fs, true );		    
	    return true;
        }
	else
	  return false;
    }
    else{
        if(KMessageBox::warningContinueCancel(0, warning_message) == KMessageBox::Continue){

            LVCreateDialog dialog(logicalVolume, false);
            dialog.exec();
            if(dialog.result() == QDialog::Accepted){
                ProcessProgress extend_lv(dialog.argumentsLV(), i18n("Extending volume..."), true);
                if( !extend_lv.exitCode() )
                    return true;
            }
        }
        return false;
    }
}

/* This class handles both the creation and extension of logical
   volumes and snapshots since both processes are so similiar. */

LVCreateDialog::LVCreateDialog(VolGroup *volumeGroup, QWidget *parent):
    KDialog(parent),
    m_vg(volumeGroup)
{

    m_lv = NULL;
    m_extend = false;
    m_snapshot = false;
    
    setCaption( i18n("Create Logical Volume") );
    m_tab_widget = new KTabWidget(this);
    m_physical_tab = createPhysicalTab();
    m_advanced_tab = createAdvancedTab();
    m_general_tab  = createGeneralTab();
    m_tab_widget->addTab(m_general_tab,  i18n("General") );
    m_tab_widget->addTab(m_physical_tab, i18n("Physical layout") );
    m_tab_widget->addTab(m_advanced_tab, i18n("Advanced") );

    setMainWidget(m_tab_widget);
}

LVCreateDialog::LVCreateDialog(LogVol *logicalVolume, bool snapshot, QWidget *parent):
    KDialog(parent),
    m_snapshot(snapshot),
    m_lv(logicalVolume)
{

    m_extend = !m_snapshot;
    m_vg = m_lv->getVolumeGroup();

    if(m_snapshot)
	setCaption( i18n("Create snapshot Volume") );
    else
	setCaption( i18n("Extend Logical Volume") );
    
    m_tab_widget = new KTabWidget(this);
    m_physical_tab = createPhysicalTab();
    m_advanced_tab = createAdvancedTab();
    m_general_tab  = createGeneralTab();
    m_tab_widget->addTab(m_general_tab,  i18n("General") );
    m_tab_widget->addTab(m_physical_tab, i18n("Physical layout") );
    m_tab_widget->addTab(m_advanced_tab, i18n("Advanced") );

    setMainWidget(m_tab_widget);
}

QWidget* LVCreateDialog::createGeneralTab()
{
     m_tag_edit = NULL;

     QWidget *general_tab = new QWidget(this);
     QHBoxLayout *general_layout = new QHBoxLayout;
     general_tab->setLayout(general_layout);
     QGroupBox *volume_box = new QGroupBox( i18n("Create new logical volume") );
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

     if( !m_extend ){
	 QHBoxLayout *name_layout = new QHBoxLayout();
	 m_name_edit = new KLineEdit();

	 QRegExp rx("[0-9a-zA-Z_\\.][-0-9a-zA-Z_\\.]*");
	 m_name_validator = new QRegExpValidator( rx, m_name_edit );
	 m_name_edit->setValidator(m_name_validator);

	 name_layout->addWidget( new QLabel( i18n("Volume name: ") ) );
	 name_layout->addWidget(m_name_edit);
	 upper_layout->insertLayout(0, name_layout);
	 m_name_is_valid = true;

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
     else {
         volume_box->setTitle( i18n("Extending volume: %1").arg(m_lv->getName()) );
	 m_name_is_valid = true;
     }

     QGroupBox *volume_size_box = new QGroupBox( i18n("Volume size") );
     QHBoxLayout *size_layout = new QHBoxLayout();
     volume_size_box->setLayout(size_layout);

     if( m_extend )
         volume_size_box->setTitle( i18n("Extend volume by:") );

     m_size_edit = new KLineEdit();
     size_layout->addWidget(m_size_edit);
     m_size_validator = new QDoubleValidator(m_size_edit); 
     m_size_edit->setValidator(m_size_validator);
     m_size_validator->setBottom(0);
     m_volume_extents = getLargestVolume() / (long long)m_vg->getExtentSize();
     m_size_edit->setText(QString("%1").arg(m_volume_extents));
     size_combo = new KComboBox();
     size_combo->insertItem(0,"Extents");
     size_combo->insertItem(1,"MiB");
     size_combo->insertItem(2,"GiB");
     size_combo->insertItem(3,"TiB");
     size_combo->setInsertPolicy(KComboBox::NoInsert);
     size_combo->setCurrentIndex(2);
     size_layout->addWidget(size_combo);
     m_size_spin = new QSpinBox();
     m_size_spin->setRange(0, 100);
     m_size_spin->setSuffix("%");
     m_size_spin->setValue(100);
     size_layout->addWidget(m_size_spin);
     upper_layout->addWidget(volume_size_box);

     QGroupBox *volume_limit_box = new QGroupBox( i18n("Volume size limit") );
     QVBoxLayout *volume_limit_layout = new QVBoxLayout();
     volume_limit_box->setLayout(volume_limit_layout);
     m_max_size_label = new QLabel();
     volume_limit_layout->addWidget(m_max_size_label);
     m_max_extents_label = new QLabel();
     volume_limit_layout->addWidget(m_max_extents_label);
     m_stripes_count_label = new QLabel();
     volume_limit_layout->addWidget(m_stripes_count_label);
     volume_limit_layout->addStretch();
     lower_layout->addWidget(volume_limit_box);

     QGroupBox *alloc_box = new QGroupBox( i18n("Allocation Policy") );
     QVBoxLayout *alloc_box_layout = new QVBoxLayout;
     normal_button     = new QRadioButton( i18n("Normal") );
     contiguous_button = new QRadioButton( i18n("Contiguous") );
     anywhere_button   = new QRadioButton( i18n("Anywhere") );
     inherited_button  = new QRadioButton( i18n("Inherited") );
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

     connect(size_combo, SIGNAL(currentIndexChanged(int)), 
	     this , SLOT(adjustSizeCombo(int)));

     connect(m_size_edit, SIGNAL(textEdited(QString)), 
	     this, SLOT(validateVolumeSize(QString)));

     connect(m_size_spin, SIGNAL(valueChanged(int)), 
	     this, SLOT(adjustSizeEdit(int)));

     setMaxSize(true);

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
        if( physical_volumes[x]->getUnused() < 1 )  // remove pvs with no free space
            physical_volumes.removeAt(x);
    }

    m_pv_checkbox = new PVCheckBox(physical_volumes, m_vg->getExtentSize());
    layout->addWidget(m_pv_checkbox);

    QVBoxLayout *striped_layout = new QVBoxLayout;
    QHBoxLayout *stripe_size_layout = new QHBoxLayout;
    QHBoxLayout *stripes_number_layout = new QHBoxLayout;
    
    m_stripe_box = new QGroupBox( i18n("Disk striping") );
    m_stripe_box->setCheckable(true);
    m_stripe_box->setChecked(false);
    m_stripe_box->setLayout(striped_layout);

    stripe_size_combo = new KComboBox();    
    for(int n = 2; (pow(2, n) * 1024) <= m_vg->getExtentSize() ; n++){
	stripe_size_combo->addItem(QString("%1").arg(pow(2, n)) + " KB");
	stripe_size_combo->setItemData(n - 2, QVariant( (int) pow(2, n) ), Qt::UserRole );
    }
    
    QLabel *stripe_size = new QLabel( i18n("Stripe Size: ") );
    m_stripes_number_spin = new QSpinBox();
    m_stripes_number_spin->setMinimum(2);
    m_stripes_number_spin->setMaximum(m_vg->getPhysVolCount());
    stripe_size_layout->addWidget(stripe_size);
    stripe_size_layout->addWidget(stripe_size_combo);
    QLabel *stripes_number = new QLabel( i18n("Number of stripes: ") );
    stripes_number_layout->addWidget(stripes_number); 
    stripes_number_layout->addWidget(m_stripes_number_spin);
    striped_layout->addLayout(stripe_size_layout);  
    striped_layout->addLayout(stripes_number_layout);

/* If we are extending a volume we try to match the striping
   of the last segment of that volume, if it was striped */

    if(m_extend){
	int seg_count = 1;
	int seg_stripe_count = 1;
	int seg_stripe_size = 4;
        QList<LogVol *>  logvols;

        if( m_lv->isMirror() ){
            logvols = m_vg->getLogicalVolumes();
            for(int x = 0; x < logvols.size(); x++){
                if(logvols[x]->isMirrorLeg() && !(logvols[x]->isMirrorLog()) ){
                    if(logvols[x]->getOrigin() == m_lv->getName()){
                        seg_count = logvols[x]->getSegmentCount();
                        seg_stripe_count = logvols[x]->getSegmentStripes(seg_count - 1);
                        seg_stripe_size = logvols[x]->getSegmentStripeSize(seg_count - 1);
                        break;
                    }
                }
            }
        }
        else{
            seg_count = m_lv->getSegmentCount();
            seg_stripe_count = m_lv->getSegmentStripes(seg_count - 1);
            seg_stripe_size = m_lv->getSegmentStripeSize(seg_count - 1);
	}

	if(seg_stripe_count > 1){
	    m_stripes_number_spin->setValue(seg_stripe_count);
	    m_stripe_box->setChecked(true);
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
    m_mirror_box->setLayout(mirror_layout);

    m_mirrored_log = new QRadioButton( i18n("Mirrored disk based log") );
    m_disk_log = new QRadioButton( i18n("Disk based log") );
    m_core_log = new QRadioButton( i18n("Memory based log") );
    m_disk_log->setChecked(true);
    mirror_layout->addWidget(m_mirrored_log);
    mirror_layout->addWidget(m_disk_log);
    mirror_layout->addWidget(m_core_log);

    QHBoxLayout *mirrors_spin_layout = new QHBoxLayout();
    
    m_mirrors_number_spin = new QSpinBox();
    m_mirrors_number_spin->setMinimum(2);
    m_mirrors_number_spin->setMaximum(m_vg->getPhysVolCount());
    QLabel *mirrors_number_label  = new QLabel( i18n("Number of mirror legs: ") );
    mirrors_spin_layout->addWidget(mirrors_number_label);
    mirrors_spin_layout->addWidget(m_mirrors_number_spin);
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

    connect(m_pv_checkbox, SIGNAL(stateChanged()), 
            this, SLOT(setAvailableSpace()));

    connect(m_stripes_number_spin, SIGNAL(valueChanged(int)), 
	    this, SLOT(setMaxSize(int)));

    connect(m_mirrors_number_spin, SIGNAL(valueChanged(int)), 
	    this, SLOT(setMaxSize(int)));

    connect(m_stripe_box, SIGNAL(toggled(bool)), 
	    this, SLOT(setMaxSize(bool)));

    connect(m_mirror_box, SIGNAL(toggled(bool)), 
	    this, SLOT(setMaxSize(bool)));

    connect(m_mirror_box, SIGNAL(toggled(bool)), 
	    this, SLOT(enableMonitoring(bool)));

    connect(m_mirrored_log, SIGNAL(toggled(bool)), 
	    this, SLOT(setMaxSize(bool)));

    connect(m_disk_log, SIGNAL(toggled(bool)), 
	    this, SLOT(setMaxSize(bool)));

    connect(m_core_log, SIGNAL(toggled(bool)), 
	    this, SLOT(setMaxSize(bool)));

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
        m_zero_check->setChecked(true);
	m_readonly_check->setChecked(false);
    
	connect(m_zero_check, SIGNAL(stateChanged(int)), 
		this ,SLOT(zeroReadonlyCheck(int)));
	connect(m_readonly_check, SIGNAL(stateChanged(int)), 
		this ,SLOT(zeroReadonlyCheck(int)));
    }
    else{
	m_zero_check->setChecked(false);
	m_zero_check->setEnabled(false);
	m_readonly_check->setChecked(false);
	m_readonly_check->setEnabled(false);
    }

    m_monitor_check = new QCheckBox( i18n("Monitor with dmeventd") );
    if(m_snapshot){
        m_monitor_check->setChecked(true);
        m_monitor_check->setEnabled(true);
	layout->addWidget(m_monitor_check);
    }
    else if(m_extend){
        m_monitor_check->setChecked(false);
        m_monitor_check->setEnabled(false);
    }
    else{
        m_monitor_check->setChecked(false);
        m_monitor_check->setEnabled(false);
	layout->addWidget(m_monitor_check);
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

void LVCreateDialog::setMaxSize(int)
{
    setMaxSize(true);
}

void LVCreateDialog::setMaxSize(bool)
{
    int stripe_count = getStripeCount();
    int mirror_count = getMirrorCount();
    int old_combo_index = size_combo->currentIndex();
    long long free_extents = getLargestVolume() / m_vg->getExtentSize(); 

    if(m_extend){    
        m_max_size_label->setText( i18n("Maximum size: %1").arg(sizeToString(getLargestVolume() + m_lv->getSize())));
        m_max_extents_label->setText( i18n("Maximum extents: %1").arg(free_extents + m_lv->getExtents()) );
    }
    else{
        m_max_size_label->setText( i18n("Maximum size: %1").arg(sizeToString( getLargestVolume() )) );
        m_max_extents_label->setText( i18n("Maximum extents: %1").arg(free_extents) );
    }

    if(!m_extend){
        if( m_mirror_box->isChecked() && !m_stripe_box->isChecked() )
            m_stripes_count_label->setText( i18n("(with %1 mirror legs)").arg(mirror_count) );
        else if( !m_mirror_box->isChecked() && m_stripe_box->isChecked() )
            m_stripes_count_label->setText( i18n("(with %1 stripes)").arg(stripe_count) );
        else if( m_mirror_box->isChecked() && m_stripe_box->isChecked()  )
            m_stripes_count_label->setText( i18n("(with %1 mirrors and %2 stripes)").arg(mirror_count).arg(stripe_count) );
        else
            m_stripes_count_label->setText( i18n("(linear volume)") );
    }
    else{
        if( mirror_count > 1 && !m_stripe_box->isChecked() )
            m_stripes_count_label->setText( i18n("(with %1 mirror legs)").arg(mirror_count) );
        else if( mirror_count < 2 && m_stripe_box->isChecked() )
            m_stripes_count_label->setText( i18n("(with %1 stripes)").arg(stripe_count) );
        else if( mirror_count > 1 && m_stripe_box->isChecked() )
            m_stripes_count_label->setText( i18n("(with %1 mirrors and %2 stripes)").arg(mirror_count).arg(stripe_count) );
        else
            m_stripes_count_label->setText( i18n("(linear volume)") );
    }

    size_combo->setCurrentIndex(0);
    size_combo->setCurrentIndex(old_combo_index);

    resetOkButton();
}

/* This adjusts the line edit according to the spin box */ 

void LVCreateDialog::adjustSizeEdit(int percentage)
{
    long long free_extents = getLargestVolume() / (long long)m_vg->getExtentSize();
    
    if(percentage == 100)
	m_volume_extents = free_extents;
    else if(percentage == 0)
	m_volume_extents = 0;
    else
	m_volume_extents = (long long)(( (double) percentage / 100) * free_extents);
    
    adjustSizeCombo( size_combo->currentIndex() );

    resetOkButton();
}

void LVCreateDialog::validateVolumeSize(QString size)
{
    int x = 0;

    const int size_combo_index = size_combo->currentIndex();

    if(m_size_validator->validate(size, x) == QValidator::Acceptable){

	if(!size_combo_index)
	    m_volume_extents = size.toLongLong();
	else
	    m_volume_extents = convertSizeToExtents( size_combo_index, size.toDouble() ); 
						 
    }
    else{
	m_volume_extents = 0;
    }

    resetOkButton();
}

void LVCreateDialog::validateVolumeName(QString name)
{
    int pos = 0;

    if( m_name_validator->validate(name, pos) == QValidator::Acceptable && name != "." && name != ".." )
	m_name_is_valid = true;
    else
	m_name_is_valid = false;
    
    resetOkButton();
}

void LVCreateDialog::resetOkButton()
{
    long long max_extents = getLargestVolume() / m_vg->getExtentSize();
    
    if( (m_volume_extents <= max_extents) && (m_volume_extents > 0) && m_name_is_valid )
	enableButtonOk(true);
    else
	enableButtonOk(false);
}

void LVCreateDialog::enableMonitoring(bool checked)
{
    if(checked){
        m_monitor_check->setChecked(true);
        m_monitor_check->setEnabled(true);
    }
    else if(!m_snapshot){
        m_monitor_check->setChecked(false);
        m_monitor_check->setEnabled(false);
    }
}

void LVCreateDialog::adjustSizeCombo(int index)
{
    long double sized;
    
    if(index){
	sized = (long double)m_volume_extents * m_vg->getExtentSize();
	if(index == 1)
	    sized /= (long double)0x100000;
	if(index == 2)
	    sized /= (long double)0x40000000;
	if(index == 3){
	    sized /= (long double)(0x100000);
	    sized /= (long double)(0x100000);
	}
	m_size_edit->setText(QString("%1").arg((double)sized));
    }
    else
	m_size_edit->setText(QString("%1").arg(m_volume_extents));
}

long long LVCreateDialog::convertSizeToExtents(int index, double size)
{
    long long extent_size;
    long double lv_size = size;
    
    extent_size = m_vg->getExtentSize();

    if(index == 1)
	lv_size *= (long double)0x100000;
    if(index == 2)
	lv_size *= (long double)0x40000000;
    if(index == 3){
	lv_size *= (long double)0x100000;
	lv_size *= (long double)0x100000;
    }

    lv_size /= extent_size;

    if (lv_size < 0)
	lv_size = 0;

    return qRound64(lv_size);
}

long long LVCreateDialog::getLargestVolume() 
{
    QList <long long> available_pv_bytes = m_pv_checkbox->getUnusedSpaceList();  
    QList <long long> stripe_pv_bytes;  
    int total_stripes = getStripeCount() * getMirrorCount();
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

    return stripe_pv_bytes[0] * getStripeCount();
}

int LVCreateDialog::getStripeCount()
{
    if(m_stripe_box->isChecked())
	return m_stripes_number_spin->value();
    else
	return 1;
}

int LVCreateDialog::getMirrorCount()
{
    if(m_extend){
        if(m_lv->isMirror())
            return m_lv->getMirrorCount();
        else
            return 1;
    }
    else if(m_mirror_box->isChecked())
	return m_mirrors_number_spin->value();
    else
	return 1;
}

void LVCreateDialog::setAvailableSpace()
{
    m_allocateable_space = m_pv_checkbox->getUnusedSpace(); 
    m_allocateable_extents = m_allocateable_space / m_vg->getExtentSize();

    resetOkButton();
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

    if(m_tag_edit){
        if( m_tag_edit->text() != "" )
            args << "--addtag" << m_tag_edit->text();
    } 

    if(m_persistent_box->isChecked()){
	args << "--persistent" << "y";
	args << "--major" << m_major_number_edit->text();
	args << "--minor" << m_minor_number_edit->text();
    }

    stripe_size = stripe_size_combo->itemData(stripe_size_combo->currentIndex(), Qt::UserRole);

    if( m_stripe_box->isChecked() || m_extend){
	args << "--stripes" << QString("%1").arg( getStripeCount() );
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
		args << "--mirrors" << QString("%1").arg( getMirrorCount() - 1 );

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

    args << "--extents" << QString("+%1").arg(m_volume_extents);
    
    if( !m_extend && !m_snapshot ){                           // create a standard volume
	program_to_run = "lvcreate";
	
	if( m_name_edit->text() != "" )
	    args << "--name" << m_name_edit->text();
	
	args << m_vg->getName();
    }
    else if( m_snapshot ){                                  // create a snapshot
	program_to_run = "lvcreate";
	
	args << "--snapshot";
	    
	if( m_name_edit->text() != "" )
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

