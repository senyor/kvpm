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


bool LVCreate(VolGroup *VolumeGroup)
{
    LVCreateDialog dialog(VolumeGroup);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
	ProcessProgress create_lv(dialog.argumentsLV(), "Creating volume...", false);
	return TRUE;
    }
    else
	return FALSE;
}

bool SnapshotCreate(LogVol *LogicalVolume)
{
    LVCreateDialog dialog(LogicalVolume, TRUE);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
	ProcessProgress create_lv(dialog.argumentsLV(), "Creating snapshot...", false);
	return TRUE;
    }
    else
	return FALSE;
}

bool LVExtend(LogVol *LogicalVolume)
{
    int error;
    QString mount_point;
    
    QString fs = LogicalVolume->getFilesystem();
    QString warning_message = "Currently only the ext2, ext3 and reiserfs file systems are";
    warning_message.append(" supported for file system extention. If this logical volume has ");
    warning_message.append(" a filesystem or data, it will need to be extended separately!");

    QString error_message = "It appears that the volume was extented but the ";
    error_message.append("filesystem was not. It will need to be extened before ");
    error_message.append("the additional space can be used.");

    if( fs == "jfs" ){
	if( LogicalVolume->isMounted() ){
	    mount_point = LogicalVolume->getMountPoints().takeFirst();
	    LVCreateDialog dialog(LogicalVolume, FALSE);
	    dialog.exec();

	    if(dialog.result() == QDialog::Accepted){
		ProcessProgress extend_lv(dialog.argumentsLV(), 
					  QString("Extending volume..."), 
					  true);
		if( !extend_lv.exitCode() ){
		    
		    error = mount( LogicalVolume->getMapperPath().toAscii().data(),
				   mount_point.toAscii().data(),
				   "",
				   MS_REMOUNT,
				   "resize" );
		    
		    if( !error )
			addMountEntryOptions( mount_point, "resize");
		    else
			KMessageBox::error(0, QString("Error number: %1  %2").arg(errno).arg(strerror(errno)));
		    
		}
		return TRUE;
	    }
	    else
		return FALSE;
	}
	else{
	    KMessageBox::error(0, "Jfs filesystems must be mounted to extend them");
	    return FALSE;
	}
    }
    else if( fs == "xfs" ){
	if( LogicalVolume->isMounted() ){

	    LVCreateDialog dialog(LogicalVolume, FALSE);
	    dialog.exec();

	    if(dialog.result() == QDialog::Accepted){
		ProcessProgress extend_lv(dialog.argumentsLV(), "Extending volume...", TRUE);
		if( !extend_lv.exitCode() ){
		    ProcessProgress extend_fs(dialog.argumentsFS(),"Extending filesystem...", TRUE);
		    if( extend_fs.exitCode() )
			KMessageBox::error(0, error_message);
		    
		}
		return TRUE;
	    }
	    else
		return FALSE;
	}
	else{
	    KMessageBox::error(0, "Xfs filesystems must be mounted to extend them");
	    return FALSE;
	}
    }
    else if( (fs != "ext2") && (fs != "ext3") && (fs != "reiserfs") ){
        if(KMessageBox::warningContinueCancel(0, warning_message) != KMessageBox::Continue)
            return FALSE;
        else{
            LVCreateDialog dialog(LogicalVolume, FALSE);
            dialog.exec();
            if(dialog.result() == QDialog::Accepted){
                ProcessProgress extend_lv(dialog.argumentsLV(), "Extending volume...", TRUE);
                return TRUE;
            }
            else
                return FALSE;
        }
    }
    else{
        LVCreateDialog dialog(LogicalVolume, FALSE);
        dialog.exec();
        if(dialog.result() == QDialog::Accepted){
	    ProcessProgress extend_lv(dialog.argumentsLV(), "Extending volume...", TRUE);
	    if( !extend_lv.exitCode() ){
		ProcessProgress extend_fs(dialog.argumentsFS(),"Extending filesystem...", TRUE);
		if( extend_fs.exitCode() )
		    KMessageBox::error(0, error_message);
	    }
            return TRUE;
	}
	else
            return FALSE;
    }
}

/* This class handles the creation and extension of logical
   volumes and snapshots since they are so similiar. */

LVCreateDialog::LVCreateDialog(VolGroup *Group, QWidget *parent):KDialog(parent)
{
    vg = Group;
    lv = NULL;
    extend = FALSE;
    snapshot = FALSE;
    
    setCaption(tr("Create Logical Volume"));

    tab_widget = new KTabWidget(this);
    physical_tab = createPhysicalTab();
    advanced_tab = createAdvancedTab();
    general_tab  = createGeneralTab();
    tab_widget->addTab(general_tab, "General");
    tab_widget->addTab(physical_tab, "Physical layout");
    tab_widget->addTab(advanced_tab, "Advanced");

    setMainWidget(tab_widget);
}

LVCreateDialog::LVCreateDialog(LogVol *LogicalVolume, bool Snapshot, QWidget *parent):KDialog(parent)
{
    snapshot = Snapshot;
    extend = !snapshot;
    
    lv = LogicalVolume;
    vg = lv->getVolumeGroup();
    if(snapshot)
	setCaption(tr("Create snapshot Volume"));
    else
	setCaption(tr("Extend Logical Volume"));
    
    tab_widget = new KTabWidget(this);
    physical_tab = createPhysicalTab();
    advanced_tab = createAdvancedTab();
    general_tab  = createGeneralTab();
    tab_widget->addTab(general_tab, "General");
    tab_widget->addTab(physical_tab, "Physical layout");
    tab_widget->addTab(advanced_tab, "Advanced");

    setMainWidget(tab_widget);

}

QWidget* LVCreateDialog::createGeneralTab()
{
     const int stripes = getStripes();
     QLabel *label;
     QString temp;

     general_tab = new QWidget(this);
     QVBoxLayout *layout = new QVBoxLayout;
     general_tab->setLayout(layout);
     
     QVBoxLayout *upper_layout = new QVBoxLayout();
     QHBoxLayout *lower_layout = new QHBoxLayout();
     layout->addStretch();
     layout->addLayout(upper_layout);
     layout->addStretch();
     layout->addLayout(lower_layout);

     if( !extend ){
	 QHBoxLayout *name_layout = new QHBoxLayout();
	 label = new QLabel("Volume name: ");
	 name_edit = new KLineEdit();
	 name_layout->addWidget(label);
	 name_layout->addWidget(name_edit);
	 upper_layout->insertLayout(0, name_layout);
     }
     else {
	 label = new QLabel("Extending volume: " + lv->getName());
	 upper_layout->addWidget(label);
     }
     
     QHBoxLayout *size_layout = new QHBoxLayout();
     size_layout->addWidget(new QLabel("Volume size: "));
     size_edit = new KLineEdit();
     size_layout->addWidget(size_edit);
     size_validator = new QDoubleValidator(size_edit); 
     size_edit->setValidator(size_validator);
     size_validator->setBottom(0);
     
     volume_size = getLargestVolume(stripes) / vg->getExtentSize();
     size_edit->setText(QString("%1").arg(volume_size));
     size_combo = new KComboBox();
     size_combo->insertItem(0,"Extents");
     size_combo->insertItem(1,"MB");
     size_combo->insertItem(2,"GB");
     size_combo->insertItem(3,"TB");
     size_combo->setInsertPolicy(KComboBox::NoInsert);
     size_combo->setCurrentIndex(2);
     size_layout->addWidget(size_combo);

     upper_layout->addLayout(size_layout);
     size_spin = new QSpinBox();
     size_spin->setRange(0, 100);
     size_spin->setSuffix("%");
     size_spin->setValue(100);
     upper_layout->addWidget(size_spin);
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
     normal_button->setChecked(TRUE);
     alloc_box_layout->addWidget(normal_button);
     alloc_box_layout->addWidget(contiguous_button);
     alloc_box_layout->addWidget(anywhere_button);
     alloc_box_layout->addWidget(inherited_button);
     alloc_box_layout->addWidget(cling_button);
     alloc_box->setLayout(alloc_box_layout);
     lower_layout->addWidget(alloc_box);

     connect(size_combo, SIGNAL(currentIndexChanged(int)), this , SLOT(adjustSizeCombo(int)));
     connect(size_edit, SIGNAL(textChanged(QString)), this, SLOT(validateSizeEdit(QString)));
     connect(size_spin, SIGNAL(valueChanged(int)), this, SLOT(adjustSizeEdit(int)));

     setMaxSize(TRUE);
     return general_tab;
}

QWidget* LVCreateDialog::createPhysicalTab()
{
    QCheckBox *temp_check;
    QString message;
    PhysVol *pv;
    int stripe_index;

    QVBoxLayout *layout = new QVBoxLayout;
    physical_tab = new QWidget(this);
    physical_tab->setLayout(layout);
    
    physical_volumes = vg->getPhysicalVolumes();
    
    QGroupBox *physical_group = new QGroupBox("Available physical volumes");
    QVBoxLayout *physical_layout = new QVBoxLayout();
    physical_group->setLayout(physical_layout);
    layout->addWidget(physical_group);
    
    for(int x = 0; x < physical_volumes.size(); x++){
	pv = physical_volumes[x];
	temp_check = new QCheckBox();

	if(pv->isAllocateable()){
	    temp_check->setEnabled(TRUE);
	    temp_check->setChecked(TRUE);
	    temp_check->setText(pv->getDeviceName() + " " + sizeToString(pv->getUnused()));
	}
	else{
	    temp_check->setEnabled(FALSE);
	    temp_check->setChecked(FALSE);
	    temp_check->setText(pv->getDeviceName() + " Not allocateable");
	}
	
	pv_checks.append(temp_check);
	physical_layout->addWidget(temp_check);

	connect(temp_check, SIGNAL(toggled(bool)), this, SLOT(calculateSpace(bool)));
	connect(temp_check, SIGNAL(toggled(bool)), this, SLOT(setMaxSize(bool)));
    }

    allocateable_space_label = new QLabel();
    allocateable_extents_label = new QLabel();
    calculateSpace(TRUE);
    physical_layout->addWidget(allocateable_space_label);
    physical_layout->addWidget(allocateable_extents_label);
    QVBoxLayout *striped_layout = new QVBoxLayout;
    QHBoxLayout *stripe_size_layout = new QHBoxLayout;
    QHBoxLayout *stripes_number_layout = new QHBoxLayout;
    
    stripe_box = new QGroupBox("Disk Striping");
    stripe_box->setCheckable(TRUE);
    stripe_box->setChecked(FALSE);
    stripe_box->setLayout(striped_layout);

    stripe_size_combo = new KComboBox();    
    for(int n = 2; (pow(2, n) * 1024) <= vg->getExtentSize() ; n++){
	stripe_size_combo->addItem(QString("%1").arg(pow(2, n)) + " KB");
	stripe_size_combo->setItemData(n - 2, QVariant( (int) pow(2, n) ), Qt::UserRole );
    }
    
    QLabel *stripe_size = new QLabel("Stripe Size: ");
    stripes_number_spin = new QSpinBox();
    stripes_number_spin->setMinimum(2);
    stripes_number_spin->setMaximum(vg->getPhysVolCount());
    stripe_size_layout->addWidget(stripe_size);
    stripe_size_layout->addWidget(stripe_size_combo);
    QLabel *stripes_number = new QLabel("Number of stripes: ");
    stripes_number_layout->addWidget(stripes_number); 
    stripes_number_layout->addWidget(stripes_number_spin);
    striped_layout->addLayout(stripe_size_layout);  
    striped_layout->addLayout(stripes_number_layout);  

/* If we are extending a volume we try to match the striping
   of the last segment of that volume, if it was striped */

    if(extend){
	const int seg_count = lv->getSegmentCount();
	const int seg_stripe_count = lv->getSegmentStripes(seg_count - 1);
	const int seg_stripe_size = lv->getSegmentStripeSize(seg_count - 1);
	
	if(seg_stripe_count > 1){
	    stripes_number_spin->setValue(seg_stripe_count);
	    stripe_box->setChecked(TRUE);
	    stripe_index = stripe_size_combo->findData( QVariant(seg_stripe_size / 1024) );
	    if(stripe_index == -1)
		stripe_index = 0;
	    stripe_size_combo->setCurrentIndex(stripe_index);
	}
    }


    layout->addWidget(stripe_box);

    connect(stripes_number_spin, SIGNAL(valueChanged(int)), this, SLOT(setMaxSize(int)));
    connect(stripe_box, SIGNAL(toggled(bool)), this, SLOT(setMaxSize(bool)));

    return physical_tab;
}

QWidget* LVCreateDialog::createAdvancedTab()
{

    QVBoxLayout *layout = new QVBoxLayout;
    advanced_tab = new QWidget(this);
    advanced_tab->setLayout(layout);
    
    persistent_box = new QGroupBox("Device numbering");
    persistent_box->setCheckable(TRUE);
    persistent_box->setChecked(FALSE);
    
    readonly_check = new QCheckBox();
    readonly_check->setText("Set Read Only");
    readonly_check->setCheckState(Qt::Unchecked);
    layout->addWidget(readonly_check);

    if( !snapshot ){
	zero_check = new QCheckBox();
	zero_check->setText("Write zeros at volume start");
	layout->addWidget(zero_check);
	zero_check->setCheckState(Qt::Checked);
	readonly_check->setEnabled(FALSE);
    
	connect(zero_check, SIGNAL(stateChanged(int)), 
		this ,SLOT(zeroReadonlyCheck(int)));
	connect(readonly_check, SIGNAL(stateChanged(int)), 
		this ,SLOT(zeroReadonlyCheck(int)));
    }
    else{
	zero_check = NULL;
	readonly_check->setEnabled(TRUE);
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
    persistent_box->setLayout(persistent_layout);
    layout->addWidget(persistent_box);
    
    layout->addStretch();

    return advanced_tab;
}

void LVCreateDialog::setMaxSize(int)
{
    setMaxSize(TRUE);
}

void LVCreateDialog::setMaxSize(bool)
{
    const int stripes = getStripes();
    long long free_extents, free_space;

    int old_combo_index = size_combo->currentIndex();
    
    free_space   = getLargestVolume(stripes);
    free_extents = free_space / vg->getExtentSize();

    max_size_label->setText("Maximum size: " + sizeToString(free_space));
    max_extents_label->setText("Maximum extents: " + QString("%1").arg(free_extents));
    if( getStripes() == 1)
	stripes_count_label->setText( QString("(linear volume)") );
    else
	stripes_count_label->setText( QString("(with %1 stripes)").arg( getStripes() ) );
    
    size_combo->setCurrentIndex(0);
    size_combo->setCurrentIndex(old_combo_index);
    validateSizeEdit( size_edit->text() );
}


/* This adjusts the line edit according to the spin box */ 

void LVCreateDialog::adjustSizeEdit(int percentage)
{
    const int stripes = getStripes();
    long long size, free_extents;
    int old_index;

    free_extents = getLargestVolume(stripes) / vg->getExtentSize();
    if(percentage == 100)
	size = free_extents;
    else if(percentage == 0)
	size = 0;
    else
	size = (long long)(( (double) percentage / 100) * free_extents);
    
    old_index = size_combo->currentIndex();
    size_combo->setCurrentIndex(0);
    size_edit->setText(QString("%1").arg(size));
    size_combo->setCurrentIndex(old_index);
}

void LVCreateDialog::validateSizeEdit(QString size)
{
    int x = 0;
    long long new_lv_size, max_extents;
    const int stripes = getStripes();
    const int size_combo_index = size_combo->currentIndex();
    
    max_extents = getLargestVolume(stripes) / vg->getExtentSize();

    if(size_validator->validate(size, x) == QValidator::Acceptable){
	if(!size_combo_index){
	    new_lv_size = size.toLongLong();
	    if( new_lv_size <= max_extents ){
		volume_size = new_lv_size;
		enableButtonOk(TRUE);
	    }
	    else
		enableButtonOk(FALSE);
	}
	else{
	    new_lv_size = convertSizeToExtents(size_combo_index, size.toDouble());
	    if( new_lv_size <= max_extents ){
		volume_size = new_lv_size;
		enableButtonOk(TRUE);
	    }
	    else
		enableButtonOk(FALSE);
	}
    }
    else
	enableButtonOk(FALSE);
}

void LVCreateDialog::adjustSizeCombo(int index)
{
    long double sized;

    if(index){
	sized = (long double)volume_size * vg->getExtentSize();
	if(index == 1)
	    sized /= (long double)0x100000;
	if(index == 2)
	    sized /= (long double)0x40000000;
	if(index == 3){
	    sized /= (long double)(0x100000);
	    sized /= (long double)(0x100000);
	}
	size_edit->setText(QString("%1").arg((double)sized));
    }
    else
	size_edit->setText(QString("%1").arg(volume_size));
}

long long LVCreateDialog::convertSizeToExtents(int index, double size)
{
    long long extent_size;
    long double lv_size = size;
    
    extent_size = vg->getExtentSize();

    if(index == 1)
	lv_size *= (long double)0x100000;
    if(index == 2)
	lv_size *= (long double)0x40000000;
    if(index == 3){
	lv_size *= (long double)(0x100000);
	lv_size *= (long double)(0x100000);
    }
    lv_size /= extent_size;
    if (lv_size < 0)
	lv_size = 0;
    return qRound64(lv_size);
}

void LVCreateDialog::calculateSpace(bool)
{
    allocateable_space = 0;
    allocateable_extents = 0;
    PhysVol *pv;
    int checked_count = 0;  // number of selected physical volumes
    
    for(int x = 0; x < pv_checks.size(); x++){
	if (pv_checks[x]->isChecked()){
	    checked_count++;
	    pv = physical_volumes[x];
	    allocateable_space += pv->getUnused();
	    allocateable_extents += (pv->getUnused()) / vg->getExtentSize();
	}
    }

/* at least one pv must always be selected */

    if (checked_count == 1){   
	for(int x = 0; x < pv_checks.size(); x++){
	    if (pv_checks[x]->isChecked())
		pv_checks[x]->setEnabled(FALSE);
	}
    }
    else{
	for(int x = 0; x < pv_checks.size(); x++){
	    if(physical_volumes[x]->isAllocateable())
		pv_checks[x]->setEnabled(TRUE);
	    else
		pv_checks[x]->setEnabled(FALSE);
	}
    }
    
    allocateable_space_label->setText("Allocateable space: " + sizeToString(allocateable_space));
    allocateable_extents_label->setText("Allocateable extents: " + QString("%1").arg(allocateable_extents));
}

long long LVCreateDialog::getLargestVolume(int stripes)
{
    QList<long long> free_list;
    long long max_size = 0;
    long long free_space;
    int list_end;
    
    if( stripes < 1 )
	return 0;
    
    for(int x = physical_volumes.size() - 1 ; x >= 0 ; x--){
	if( ( physical_volumes[x]->getUnused() > 0 ) && ( pv_checks[x]->isChecked() ) )    
	    free_list.append( physical_volumes[x]->getUnused() );
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

int LVCreateDialog::getStripes()
{
    if(stripe_box->isChecked())
	return stripes_number_spin->value();
    else
	return 1;
}

void LVCreateDialog::zeroReadonlyCheck(int)
{
    if( !snapshot ){             // zero_check = NULL if this is a snapshot
	if (zero_check->isChecked()){
	    readonly_check->setChecked(FALSE);
	    readonly_check->setEnabled(FALSE);
	}
	else
	    readonly_check->setEnabled(TRUE);
	
	if (readonly_check->isChecked()){
	    zero_check->setChecked(FALSE);
	    zero_check->setEnabled(FALSE);
	}
	else
	    zero_check->setEnabled(TRUE);
    }
    else
	readonly_check->setEnabled(TRUE);
}

/* Here we create a stringlist of arguments based on all
   the options that the user chose in the dialog. */

QStringList LVCreateDialog::argumentsLV()
{
    QString program_to_run;
    QStringList args;

    if(persistent_box->isChecked()){
	args << "--persistent" << "y";
	args << "--major" << major_number_edit->text();
	args << "--minor" << minor_number_edit->text();
    }

    args << "--stripes" << QString("%1").arg( getStripes() );
    
    if( !extend ){
	if ( readonly_check->isChecked() )
	    args << "--permission" << "r" ;
	else 
	    args << "--permission" << "rw" ;

	if ( !inherited_button->isChecked() ){        // "inherited" is what we get if
	    args << "--alloc";                      // we don't pass "--alloc" at all
	    if ( contiguous_button->isChecked() )     // passing "--alloc" "inherited"
		args << "contiguous" ;              // doesn't work
	    else if ( anywhere_button->isChecked() )
		args << "anywhere" ;
	    else if ( cling_button->isChecked() )
		args << "cling" ;
	    else
		args << "normal" ;
	}
    }
	
    if( !snapshot && !extend ){
	if(zero_check->isChecked())
	    args << "--zero" << "y";
	else
	    args << "--zero" << "n";
    }
    
    args << "--extents" << QString("+%1").arg(volume_size);
    
    if( !extend && !snapshot ){                           // create a standard volume
	program_to_run = "/sbin/lvcreate";
	
	if( name_edit->text() != "" )
	    args << "--name" << (QString)name_edit->text();
	
	args << vg->getName();
    }
    else if( snapshot ){                                  // create a snapshot
	program_to_run = "/sbin/lvcreate";
	
	args << "--snapshot";
	    
	if( name_edit->text() != "" )
	    args << "--name" << (QString)name_edit->text() ;
	args << lv->getFullName();
    }
    else{                                               // extend the current volume
	program_to_run = "/sbin/lvextend";
	args <<	lv->getFullName();
    }

    for(int x = 0; x < pv_checks.size(); x++)
	if ( pv_checks[x]->isChecked() )
	    args << physical_volumes[x]->getDeviceName();

    args.prepend(program_to_run);
    return args;
}

QStringList LVCreateDialog::argumentsFS()
{
    QStringList temp;
    QStringList args;
    QString fs = lv->getFilesystem();
    QString mount_point;
    
    if( lv->getMountPoints().size() )
	mount_point = lv->getMountPoints().takeFirst();
    else
	qDebug() << "xfs was not mounted!";
    
    if( fs == "xfs" ){
	args << "xfs_growfs" << mount_point;
    }
    else if( (fs == "ext2") || (fs == "ext3") ){
        args << "/sbin/resize2fs" << "-f"
	     << "/dev/" + vg->getName() + "/" + lv->getName();
    }
    else if(fs == "reiserfs"){
        args << "/sbin/resize_reiserfs"
	     << "/dev/" + vg->getName() + "/" + lv->getName();
    }
    return args;
}
