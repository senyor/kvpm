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


#include <sys/mount.h>
#include <errno.h>
#include <cmath>

#include <KMessageBox>
#include <QtGui>

#include "logvol.h"
#include "lvcreate.h"
#include "mountentry.h"
#include "physvol.h"
#include "processprogress.h"
#include "sizetostring.h"
#include "volgroup.h"


bool LVCreate(VolGroup *volumeGroup)
{
    LVCreateDialog dialog(volumeGroup);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
	ProcessProgress create_lv(dialog.argumentsLV(), "Creating volume...", false);
	return true;
    }
    else
	return false;
}

bool SnapshotCreate(LogVol *logicalVolume)
{
    LVCreateDialog dialog(logicalVolume, true);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
	ProcessProgress create_lv(dialog.argumentsLV(), "Creating snapshot...", false);
	return true;
    }
    else
	return false;
}

bool LVExtend(LogVol *logicalVolume)
{
    int error;
    QString mount_point;
    
    QString fs = logicalVolume->getFilesystem();
    QString warning_message = "Currently only the ext2, ext3 and reiserfs file systems are";
    warning_message.append(" supported for file system extention. If this logical volume has ");
    warning_message.append(" a filesystem or data, it will need to be extended separately!");

    QString error_message = "It appears that the volume was extented but the ";
    error_message.append("filesystem was not. It will need to be extened before ");
    error_message.append("the additional space can be used.");

    if( fs == "jfs" ){
	if( logicalVolume->isMounted() ){
	    mount_point = logicalVolume->getMountPoints().takeFirst();
	    LVCreateDialog dialog(logicalVolume, false);
	    dialog.exec();

	    if(dialog.result() == QDialog::Accepted){
		ProcessProgress extend_lv(dialog.argumentsLV(), 
					  QString("Extending volume..."), 
					  true);
		if( !extend_lv.exitCode() ){
		    
		    error = mount( logicalVolume->getMapperPath().toAscii().data(),
				   mount_point.toAscii().data(),
				   "",
				   MS_REMOUNT,
				   "resize" );
		    
		    if( !error )
			addMountEntryOptions( mount_point, "resize");
		    else
			KMessageBox::error(0, QString("Error number: %1  %2").arg(errno).arg(strerror(errno)));
		    
		}
		return true;
	    }
	    else
		return false;
	}
	else{
	    KMessageBox::error(0, "Jfs filesystems must be mounted to extend them");
	    return false;
	}
    }
    else if( fs == "xfs" ){
	if( logicalVolume->isMounted() ){

	    LVCreateDialog dialog(logicalVolume, false);
	    dialog.exec();

	    if(dialog.result() == QDialog::Accepted){
		ProcessProgress extend_lv(dialog.argumentsLV(), "Extending volume...", true);
		if( !extend_lv.exitCode() ){
		    ProcessProgress extend_fs(dialog.argumentsFS(),"Extending filesystem...", true);
		    if( extend_fs.exitCode() )
			KMessageBox::error(0, error_message);
		    
		}
		return true;
	    }
	    else
		return false;
	}
	else{
	    KMessageBox::error(0, "Xfs filesystems must be mounted to extend them");
	    return false;
	}
    }
    else if( (fs != "ext2") && (fs != "ext3") && (fs != "reiserfs") ){
        if(KMessageBox::warningContinueCancel(0, warning_message) != KMessageBox::Continue)
            return false;
        else{
            LVCreateDialog dialog(logicalVolume, false);
            dialog.exec();
            if(dialog.result() == QDialog::Accepted){
                ProcessProgress extend_lv(dialog.argumentsLV(), "Extending volume...", true);
                return true;
            }
            else
                return false;
        }
    }
    else{
        LVCreateDialog dialog(logicalVolume, false);
        dialog.exec();
        if(dialog.result() == QDialog::Accepted){
	    ProcessProgress extend_lv(dialog.argumentsLV(), "Extending volume...", true);
	    if( !extend_lv.exitCode() ){
		ProcessProgress extend_fs(dialog.argumentsFS(),"Extending filesystem...", true);
		if( extend_fs.exitCode() )
		    KMessageBox::error(0, error_message);
	    }
            return true;
	}
	else
            return false;
    }
}

/* This class handles the creation and extension of logical
   volumes and snapshots since they are so similiar. */

LVCreateDialog::LVCreateDialog(VolGroup *volumeGroup, QWidget *parent):
    KDialog(parent),
    m_vg(volumeGroup)
{

    m_lv = NULL;
    m_extend = false;
    m_snapshot = false;
    
    setCaption(tr("Create Logical Volume"));

    m_tab_widget = new KTabWidget(this);
    m_physical_tab = createPhysicalTab();
    m_advanced_tab = createAdvancedTab();
    m_general_tab  = createGeneralTab();
    m_tab_widget->addTab(m_general_tab, "General");
    m_tab_widget->addTab(m_physical_tab, "Physical layout");
    m_tab_widget->addTab(m_advanced_tab, "Advanced");

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
	setCaption(tr("Create snapshot Volume"));
    else
	setCaption(tr("Extend Logical Volume"));
    
    m_tab_widget = new KTabWidget(this);
    m_physical_tab = createPhysicalTab();
    m_advanced_tab = createAdvancedTab();
    m_general_tab  = createGeneralTab();
    m_tab_widget->addTab(m_general_tab, "General");
    m_tab_widget->addTab(m_physical_tab, "Physical layout");
    m_tab_widget->addTab(m_advanced_tab, "Advanced");

    setMainWidget(m_tab_widget);

}

QWidget* LVCreateDialog::createGeneralTab()
{
     QLabel *label;
     QString temp;

     m_general_tab = new QWidget(this);
     QVBoxLayout *layout = new QVBoxLayout;
     m_general_tab->setLayout(layout);
     
     QVBoxLayout *upper_layout = new QVBoxLayout();
     QHBoxLayout *lower_layout = new QHBoxLayout();
     layout->addStretch();
     layout->addLayout(upper_layout);
     layout->addStretch();
     layout->addLayout(lower_layout);

     if( !m_extend ){
	 QHBoxLayout *name_layout = new QHBoxLayout();
	 label = new QLabel("Volume name: ");
	 name_edit = new KLineEdit();
	 name_layout->addWidget(label);
	 name_layout->addWidget(name_edit);
	 upper_layout->insertLayout(0, name_layout);
     }
     else {
	 label = new QLabel("Extending volume: " + m_lv->getName());
	 upper_layout->addWidget(label);
     }
     
     QHBoxLayout *size_layout = new QHBoxLayout();
     size_layout->addWidget(new QLabel("Volume size: "));
     m_size_edit = new KLineEdit();
     size_layout->addWidget(m_size_edit);
     size_validator = new QDoubleValidator(m_size_edit); 
     m_size_edit->setValidator(size_validator);
     size_validator->setBottom(0);
     
     m_volume_size = getLargestVolume( getStripeCount() ) / m_vg->getExtentSize();
     m_size_edit->setText(QString("%1").arg(m_volume_size));
     size_combo = new KComboBox();
     size_combo->insertItem(0,"Extents");
     size_combo->insertItem(1,"MB");
     size_combo->insertItem(2,"GB");
     size_combo->insertItem(3,"TB");
     size_combo->setInsertPolicy(KComboBox::NoInsert);
     size_combo->setCurrentIndex(2);
     size_layout->addWidget(size_combo);

     upper_layout->addLayout(size_layout);
     m_size_spin = new QSpinBox();
     m_size_spin->setRange(0, 100);
     m_size_spin->setSuffix("%");
     m_size_spin->setValue(100);
     upper_layout->addWidget(m_size_spin);
     QGroupBox *volume_limit_box = new QGroupBox("Volume size limit");
     QVBoxLayout *volume_limit_layout = new QVBoxLayout();
     volume_limit_box->setLayout(volume_limit_layout);
     max_size_label = new QLabel();
     volume_limit_layout->addWidget(max_size_label);
     max_extents_label = new QLabel();
     volume_limit_layout->addWidget(max_extents_label);
     stripes_count_label = new QLabel();
     volume_limit_layout->addWidget(stripes_count_label);
     volume_limit_layout->addStretch();
     lower_layout->addWidget(volume_limit_box);

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
     lower_layout->addWidget(alloc_box);

     connect(size_combo, SIGNAL(currentIndexChanged(int)), 
	     this , SLOT(adjustSizeCombo(int)));

     connect(m_size_edit, SIGNAL(textEdited(QString)), 
	     this, SLOT(validateVolumeSize(QString)));

     connect(m_size_spin, SIGNAL(valueChanged(int)), 
	     this, SLOT(adjustSizeEdit(int)));

     setMaxSize(true);
     return m_general_tab;
}

QWidget* LVCreateDialog::createPhysicalTab()
{
    QCheckBox *temp_check;
    QString message;
    PhysVol *pv;
    int stripe_index;

    QVBoxLayout *layout = new QVBoxLayout;
    m_physical_tab = new QWidget(this);
    m_physical_tab->setLayout(layout);
    
    m_physical_volumes = m_vg->getPhysicalVolumes();
    
    QGroupBox *physical_group = new QGroupBox("Available physical volumes");
    QVBoxLayout *physical_layout = new QVBoxLayout();
    physical_group->setLayout(physical_layout);
    layout->addWidget(physical_group);
    
    for(int x = 0; x < m_physical_volumes.size(); x++){
	pv = m_physical_volumes[x];
	temp_check = new QCheckBox();

	if(pv->isAllocateable()){
	    temp_check->setEnabled(true);
	    temp_check->setChecked(true);
	    temp_check->setText(pv->getDeviceName() + " " + sizeToString(pv->getUnused()));
	}
	else{
	    temp_check->setEnabled(false);
	    temp_check->setChecked(false);
	    temp_check->setText(pv->getDeviceName() + " Not allocateable");
	}
	
	m_pv_checks.append(temp_check);
	physical_layout->addWidget(temp_check);

	connect(temp_check, SIGNAL(toggled(bool)), 
		this, SLOT(calculateSpace(bool)));

	connect(temp_check, SIGNAL(toggled(bool)), 
		this, SLOT(setMaxSize(bool)));
    }

    allocateable_space_label = new QLabel();
    allocateable_extents_label = new QLabel();
    calculateSpace(true);
    physical_layout->addWidget(allocateable_space_label);
    physical_layout->addWidget(allocateable_extents_label);
    QVBoxLayout *striped_layout = new QVBoxLayout;
    QHBoxLayout *stripe_size_layout = new QHBoxLayout;
    QHBoxLayout *stripes_number_layout = new QHBoxLayout;
    
    m_stripe_box = new QGroupBox("Disk striping");
    m_stripe_box->setCheckable(true);
    m_stripe_box->setChecked(false);
    m_stripe_box->setLayout(striped_layout);

    stripe_size_combo = new KComboBox();    
    for(int n = 2; (pow(2, n) * 1024) <= m_vg->getExtentSize() ; n++){
	stripe_size_combo->addItem(QString("%1").arg(pow(2, n)) + " KB");
	stripe_size_combo->setItemData(n - 2, QVariant( (int) pow(2, n) ), Qt::UserRole );
    }
    
    QLabel *stripe_size = new QLabel("Stripe Size: ");
    m_stripes_number_spin = new QSpinBox();
    m_stripes_number_spin->setMinimum(2);
    m_stripes_number_spin->setMaximum(m_vg->getPhysVolCount());
    stripe_size_layout->addWidget(stripe_size);
    stripe_size_layout->addWidget(stripe_size_combo);
    QLabel *stripes_number = new QLabel("Number of stripes: ");
    stripes_number_layout->addWidget(stripes_number); 
    stripes_number_layout->addWidget(m_stripes_number_spin);
    striped_layout->addLayout(stripe_size_layout);  
    striped_layout->addLayout(stripes_number_layout);

/* If we are extending a volume we try to match the striping
   of the last segment of that volume, if it was striped */

    if(m_extend){
	const int seg_count = m_lv->getSegmentCount();
	const int seg_stripe_count = m_lv->getSegmentStripes(seg_count - 1);
	const int seg_stripe_size = m_lv->getSegmentStripeSize(seg_count - 1);
	
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
    m_mirror_box = new QGroupBox("Disk mirrors");
    m_mirror_box->setCheckable(true);
    m_mirror_box->setChecked(false);
    m_mirror_box->setLayout(mirror_layout);

    m_disk_log = new QRadioButton("Disk based log");
    m_core_log = new QRadioButton("Memory based log");
    m_disk_log->setChecked(true);
    mirror_layout->addWidget(m_disk_log);
    mirror_layout->addWidget(m_core_log);

    QHBoxLayout *mirrors_spin_layout = new QHBoxLayout();
    
    m_mirrors_number_spin = new QSpinBox();
    m_mirrors_number_spin->setMinimum(2);
    m_mirrors_number_spin->setMaximum(m_vg->getPhysVolCount());
    QLabel *mirrors_number_label  = new QLabel("Number of mirror legs: ");
    mirrors_spin_layout->addWidget(mirrors_number_label);
    mirrors_spin_layout->addWidget(m_mirrors_number_spin);
    mirror_layout->addLayout(mirrors_spin_layout);
    
    layout->addWidget(m_stripe_box);

    if( (!m_extend) && (!m_snapshot) )
	layout->addWidget(m_mirror_box);
    else
	m_mirror_box->setEnabled(false);

    connect(m_stripes_number_spin, SIGNAL(valueChanged(int)), 
	    this, SLOT(setMaxSize(int)));

    connect(m_mirrors_number_spin, SIGNAL(valueChanged(int)), 
	    this, SLOT(setMaxSize(int)));

    connect(m_stripe_box, SIGNAL(toggled(bool)), 
	    this, SLOT(setMaxSize(bool)));

    connect(m_mirror_box, SIGNAL(toggled(bool)), 
	    this, SLOT(setMaxSize(bool)));

    connect(m_stripe_box, SIGNAL(toggled(bool)), 
	    this, SLOT(enableMirrorBox(bool)));

    connect(m_mirror_box, SIGNAL(toggled(bool)), 
	    this, SLOT(enableStripeBox(bool)));

    return m_physical_tab;
}

QWidget* LVCreateDialog::createAdvancedTab()
{

    QVBoxLayout *layout = new QVBoxLayout;
    m_advanced_tab = new QWidget(this);
    m_advanced_tab->setLayout(layout);
    
    m_persistent_box = new QGroupBox("Device numbering");
    m_persistent_box->setCheckable(true);
    m_persistent_box->setChecked(false);
    
    readonly_check = new QCheckBox();
    readonly_check->setText("Set Read Only");
    readonly_check->setCheckState(Qt::Unchecked);
    layout->addWidget(readonly_check);

    if( !m_snapshot ){
	zero_check = new QCheckBox();
	zero_check->setText("Write zeros at volume start");
	layout->addWidget(zero_check);
	zero_check->setCheckState(Qt::Checked);
	readonly_check->setEnabled(false);
    
	connect(zero_check, SIGNAL(stateChanged(int)), 
		this ,SLOT(zeroReadonlyCheck(int)));
	connect(readonly_check, SIGNAL(stateChanged(int)), 
		this ,SLOT(zeroReadonlyCheck(int)));
    }
    else{
	zero_check = NULL;
	readonly_check->setEnabled(true);
    }
    
    QVBoxLayout *persistent_layout = new QVBoxLayout;
    QHBoxLayout *minor_number_layout = new QHBoxLayout;
    QHBoxLayout *major_number_layout = new QHBoxLayout;
    minor_number_edit = new KLineEdit();
    major_number_edit = new KLineEdit();
    QLabel *minor_number = new QLabel("Device minor number: ");
    QLabel *major_number = new QLabel("Device major number: ");
    major_number_layout->addWidget(major_number);
    major_number_layout->addWidget(major_number_edit);
    minor_number_layout->addWidget(minor_number);
    minor_number_layout->addWidget(minor_number_edit);
    persistent_layout->addLayout(major_number_layout);
    persistent_layout->addLayout(minor_number_layout);
    QIntValidator *minor_validator = new QIntValidator(minor_number_edit); 
    QIntValidator *major_validator = new QIntValidator(major_number_edit); 
    minor_validator->setBottom(0);
    major_validator->setBottom(0);
    minor_number_edit->setValidator(minor_validator);
    major_number_edit->setValidator(major_validator);
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

    long long free_extents, 
	      free_space;

    int old_combo_index = size_combo->currentIndex();

    if( m_stripe_box->isChecked() ){
	free_space   = getLargestVolume(stripe_count);
	free_extents = free_space / m_vg->getExtentSize();

	max_size_label->setText("Maximum size: " + sizeToString(free_space));
	max_extents_label->setText("Maximum extents: " + QString("%1").arg(free_extents));

	stripes_count_label->setText( QString("(with %1 stripes)").arg(stripe_count) );
    
	size_combo->setCurrentIndex(0);
	size_combo->setCurrentIndex(old_combo_index);
    }
    else if( m_mirror_box->isChecked() ){
	free_space   = getLargestMirror( mirror_count, m_disk_log->isChecked() );
	free_extents = free_space / m_vg->getExtentSize();

	max_size_label->setText("Maximum size: " + sizeToString(free_space));
	max_extents_label->setText("Maximum extents: " + QString("%1").arg(free_extents));

	stripes_count_label->setText( QString("(with %1 mirror legs)").arg(mirror_count) );
    
	size_combo->setCurrentIndex(0);
	size_combo->setCurrentIndex(old_combo_index);
    }
    else{
	free_space   = getLargestVolume(1);
	free_extents = free_space / m_vg->getExtentSize();

	max_size_label->setText("Maximum size: " + sizeToString(free_space));
	max_extents_label->setText("Maximum extents: " + QString("%1").arg(free_extents));

	stripes_count_label->setText( QString("(linear volume)") );
    
	size_combo->setCurrentIndex(0);
	size_combo->setCurrentIndex(old_combo_index);
    }

    validateVolumeSize();
}

void LVCreateDialog::enableStripeBox(bool toggle_state)
{
    if(toggle_state){
	m_stripe_box->setChecked(false);
	m_stripe_box->setEnabled(false);
    }
    else
	m_stripe_box->setEnabled(true);
}


void LVCreateDialog::enableMirrorBox(bool toggle_state)
{
    if(toggle_state){
	m_mirror_box->setChecked(false);
	m_mirror_box->setEnabled(false);
    }
    else
	m_mirror_box->setEnabled(true);
}


/* This adjusts the line edit according to the spin box */ 

void LVCreateDialog::adjustSizeEdit(int percentage)
{
    long long free_extents;
	      
    if( m_stripe_box->isChecked() )
	free_extents = getLargestVolume( getStripeCount() );
    else if( m_mirror_box->isChecked() )
	free_extents = getLargestMirror( getMirrorCount(), m_disk_log->isChecked() );
    else
	free_extents = getLargestVolume(1);
    
    free_extents /= m_vg->getExtentSize();

    if(percentage == 100)
	m_volume_size = free_extents;
    else if(percentage == 0)
	m_volume_size = 0;
    else
	m_volume_size = (long long)(( (double) percentage / 100) * free_extents);
    
    adjustSizeCombo( size_combo->currentIndex() );

    validateVolumeSize();
}

void LVCreateDialog::validateVolumeSize(QString size)
{
    int x = 0;

    const int size_combo_index = size_combo->currentIndex();

    if(size_validator->validate(size, x) == QValidator::Acceptable){

	if(!size_combo_index)
	    m_volume_size = size.toLongLong();
	else
	    m_volume_size = convertSizeToExtents( size_combo_index, size.toDouble() ); 
						 
    }
    else{
	m_volume_size = 0;
	enableButtonOk(false);
    }

    validateVolumeSize();
}

void LVCreateDialog::validateVolumeSize()
{
    long long max_extents;
	      
    if( m_stripe_box->isChecked() ){    
	max_extents = getLargestVolume( getStripeCount() ) / m_vg->getExtentSize();
    }
    else if( m_mirror_box->isChecked() ){
	max_extents = getLargestMirror( m_mirrors_number_spin->value(), 
					m_disk_log->isChecked() ) / m_vg->getExtentSize();
    }
    else{
	max_extents = getLargestVolume(1) / m_vg->getExtentSize();
    }
    
    if( (m_volume_size <= max_extents) && (m_volume_size > 0) )
	enableButtonOk(true);
    else
	enableButtonOk(false);

}


void LVCreateDialog::adjustSizeCombo(int index)
{
    long double sized;
    
    if(index){
	sized = (long double)m_volume_size * m_vg->getExtentSize();
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
	m_size_edit->setText(QString("%1").arg(m_volume_size));
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

void LVCreateDialog::calculateSpace(bool)
{
    m_allocateable_space = 0;
    m_allocateable_extents = 0;
    PhysVol *pv;
    int checked_count = 0;  // number of selected physical volumes
    
    for(int x = 0; x < m_pv_checks.size(); x++){
	if (m_pv_checks[x]->isChecked()){
	    checked_count++;
	    pv = m_physical_volumes[x];
	    m_allocateable_space += pv->getUnused();
	    m_allocateable_extents += (pv->getUnused()) / m_vg->getExtentSize();
	}
    }

/* at least one pv must always be selected */

    if (checked_count == 1){   
	for(int x = 0; x < m_pv_checks.size(); x++){
	    if (m_pv_checks[x]->isChecked())
		m_pv_checks[x]->setEnabled(false);
	}
    }
    else{
	for(int x = 0; x < m_pv_checks.size(); x++){
	    if(m_physical_volumes[x]->isAllocateable())
		m_pv_checks[x]->setEnabled(true);
	    else
		m_pv_checks[x]->setEnabled(false);
	}
    }
    
    allocateable_space_label->setText("Allocateable space: " + sizeToString(m_allocateable_space));
    allocateable_extents_label->setText("Allocateable extents: " + QString("%1").arg(m_allocateable_extents));
}

/* Determine just how big a stripe set or linear volume we can create */

long long LVCreateDialog::getLargestVolume(int stripes)
{
    QList<long long> free_list;
    long long max_size = 0;
    long long free_space;
    int list_end;
    
    if( stripes < 1 )
	return 0;
    
    for(int x = m_physical_volumes.size() - 1 ; x >= 0 ; x--){
	if( ( m_physical_volumes[x]->getUnused() > 0 ) && ( m_pv_checks[x]->isChecked() ) )    
	    free_list.append( m_physical_volumes[x]->getUnused() );
    }

    if( stripes == 1 ){
	for(int x = free_list.size() - 1 ; x >= 0 ; x--)
	    max_size += free_list[x];
	return max_size;
    }
    
    while( stripes <= free_list.size() ){

	qSort(free_list.begin(), free_list.end());

	list_end = free_list.size() - 1;

	for(int x = list_end; x > list_end - stripes; x--){
	    free_space = free_list[ (list_end - stripes) + 1];
	    max_size += free_space;
	    free_list[x] -= free_space;
	}

	for(int x = free_list.size() - 1 ; x >= 0 ; x--)
	    if( free_list[x] <= 0)
		free_list.removeAt(x);
    }

    return max_size;
}

/* Determine just how big a mirror set we can create 
   with a given number of legs and a log, or no log */

long long LVCreateDialog::getLargestMirror(int mirrors, bool disk_log)
{
    QList<long long> free_list;
    long long max_size = 0;

    for(int x = m_physical_volumes.size() - 1 ; x >= 0 ; x--){
	if( ( m_physical_volumes[x]->getUnused() > 0 ) && ( m_pv_checks[x]->isChecked() ) )    
	    free_list.append( m_physical_volumes[x]->getUnused() );
    }

    qSort(free_list.begin(), free_list.end());

    if( !disk_log){
	if( mirrors > free_list.size() )
	    max_size = 0;
	else
	    max_size = free_list[ free_list.size() - mirrors ];
    }
    else{
	if( (mirrors + 1) > free_list.size() )
	    max_size = 0;
	else
	    max_size = free_list[ free_list.size() - mirrors ];
    }
    
    return max_size;
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
    if(m_mirror_box->isChecked())
	return m_mirrors_number_spin->value();
    else
	return 1;
}

void LVCreateDialog::zeroReadonlyCheck(int)
{
    if( !m_snapshot ){             // zero_check = NULL if this is a snapshot
	if (zero_check->isChecked()){
	    readonly_check->setChecked(false);
	    readonly_check->setEnabled(false);
	}
	else
	    readonly_check->setEnabled(true);
	
	if (readonly_check->isChecked()){
	    zero_check->setChecked(false);
	    zero_check->setEnabled(false);
	}
	else
	    zero_check->setEnabled(true);
    }
    else
	readonly_check->setEnabled(true);
}

/* Here we create a stringlist of arguments based on all
   the options that the user chose in the dialog. */

QStringList LVCreateDialog::argumentsLV()
{
    QString program_to_run;
    QStringList args;

    if(m_persistent_box->isChecked()){
	args << "--persistent" << "y";
	args << "--major" << major_number_edit->text();
	args << "--minor" << minor_number_edit->text();
    }

    if( m_stripe_box->isChecked() )
	args << "--stripes" << QString("%1").arg( getStripeCount() );
    
    if( !m_extend ){
	if ( readonly_check->isChecked() )
	    args << "--permission" << "r" ;
	else 
	    args << "--permission" << "rw" ;

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
    }
	
    if( !m_snapshot && !m_extend ){

	if( zero_check->isChecked() )
	    args << "--zero" << "y";
	else
	    args << "--zero" << "n";

	if( m_mirror_box->isChecked() )
	    args << "--mirrors" << QString("%1").arg( getMirrorCount() - 1 );
    }
    
    args << "--extents" << QString("+%1").arg(m_volume_size);
    
    if( !m_extend && !m_snapshot ){                           // create a standard volume
	program_to_run = "lvcreate";
	
	if( name_edit->text() != "" )
	    args << "--name" << (QString)name_edit->text();
	
	args << m_vg->getName();
    }
    else if( m_snapshot ){                                  // create a snapshot
	program_to_run = "lvcreate";
	
	args << "--snapshot";
	    
	if( name_edit->text() != "" )
	    args << "--name" << (QString)name_edit->text() ;
	args << m_lv->getFullName();
    }
    else{                                               // extend the current volume
	program_to_run = "lvextend";
	args <<	m_lv->getFullName();
    }

    for(int x = 0; x < m_pv_checks.size(); x++)
	if ( m_pv_checks[x]->isChecked() )
	    args << m_physical_volumes[x]->getDeviceName();

    args.prepend(program_to_run);
    return args;
}

QStringList LVCreateDialog::argumentsFS()
{
    QStringList temp;
    QStringList args;
    QString fs = m_lv->getFilesystem();
    QString mount_point;
    
    if( m_lv->getMountPoints().size() )
	mount_point = m_lv->getMountPoints().takeFirst();
    else
	qDebug() << "xfs was not mounted!";
    
    if( fs == "xfs" ){
	args << "xfs_growfs" << mount_point;
    }
    else if( (fs == "ext2") || (fs == "ext3") ){
        args << "resize2fs" << "-f"
	     << "/dev/" + m_vg->getName() + "/" + m_lv->getName();
    }
    else if(fs == "reiserfs"){
        args << "resize_reiserfs"
	     << "/dev/" + m_vg->getName() + "/" + m_lv->getName();
    }

    return args;
}
