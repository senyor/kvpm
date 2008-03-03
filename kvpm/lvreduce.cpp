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
#include "lvreduce.h"
#include "processprogress.h"
#include "sizetostring.h"


bool LVReduce(LogVol *LogicalVolume)
{
    QString fs, state, type;
    QStringList fs_args, lv_args;
    
    fs = LogicalVolume->getFilesystem();
    QString warning_message = "Currently only the ext2 and ext3  file systems are supported";
    warning_message.append("for file system reduction. If this logical volume is reduced");
    warning_message.append(" any data it contains will be lost!");
	
    state = LogicalVolume->getState();
    QString warning_message2 = "If this <b>Inactive</b> logical volume is reduced";
    warning_message2.append(" any data it contains will be lost!");

    type = LogicalVolume->getType();

    if( type == "Mirror"){
	KMessageBox::error(0, "Resizing mirrors is not supported yet");
	return FALSE;
    }
    else if( type == "Valid Snap"){
	LVReduceDialog dialog(LogicalVolume);
	dialog.exec();
	if(dialog.result() == QDialog::Accepted){
	    ProcessProgress reduce_lv(dialog.argumentsLV(), 
				      QString("Reducing volume..."), 
				      true);
	    return TRUE;
	}
	return TRUE;
    }
    else if( type == "Mirror"){
	KMessageBox::error(0, "Resizing mirrors is not supported yet");
	return FALSE;
    }
    else if( type == "Origin"){
	KMessageBox::error(0, "Resizing snapshot origin is not supported yet");
	return FALSE;
    }
    else if( LogicalVolume->isMounted() ){
	KMessageBox::error(0, "The filesystem must be unmounted first");
	return FALSE;
    }
    else if( state != "Active" ){
	if(KMessageBox::warningContinueCancel(0, warning_message2) != KMessageBox::Continue)
	    return FALSE;
	else{
	    LVReduceDialog dialog(LogicalVolume);
	    dialog.exec();
	    if(dialog.result() == QDialog::Accepted){
		ProcessProgress reduce_lv(dialog.argumentsLV(), "Reducing volume...", TRUE);
		return TRUE;
	    }
	    else
		return FALSE;
	}
    }
    else if( (fs != "ext2") && (fs != "ext3") ){
	if(KMessageBox::warningContinueCancel(0, warning_message) != KMessageBox::Continue)
	    return FALSE;
	else{
	    LVReduceDialog dialog(LogicalVolume);
	    dialog.exec();
	    if(dialog.result() == QDialog::Accepted){
		ProcessProgress reduce_lv(dialog.argumentsLV(), "Reducing volume...", TRUE);
		return TRUE;
	    }
	    else
		return FALSE;
	}
    }
    else{
	LVReduceDialog dialog(LogicalVolume);
	dialog.exec();
	if(dialog.result() == QDialog::Accepted){
	    ProcessProgress reduce_fs(dialog.argumentsFS(), "Reducing filesystem...", TRUE);
	    if( !reduce_fs.exitCode() )
		ProcessProgress reduce_lv(dialog.argumentsLV(), "Reducing volume...", TRUE);
	    return TRUE;
	}
	else
	    return FALSE;
    }
}


LVReduceDialog::LVReduceDialog(LogVol *LogicalVolume, QWidget *parent) : KDialog(parent)
{
    lv = LogicalVolume;
    vg = lv->getVolumeGroup();
    setWindowTitle(tr("Reduce Logical Volume"));

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    QLabel *ext_size_label, *current_size_label;
    
    current_size = lv->getSize();
    ext_size_label = new QLabel("Extent size: " + sizeToString(vg->getExtentSize()));
    current_size_label = new QLabel("Current size: " + sizeToString(current_size));

    layout->addWidget(ext_size_label);
    layout->addWidget(current_size_label);
    QHBoxLayout *size_layout = new QHBoxLayout();
    size_layout->addWidget(new QLabel("New size: "));
    
    size_edit = new QLineEdit();
    size_validator = new QDoubleValidator(size_edit);
    size_validator->setBottom(0.0);
    size_edit->setValidator(size_validator);
    connect(size_edit, SIGNAL(textEdited(QString)),
	    this, SLOT(validateInput(QString)));
    
    size_combo = new QComboBox();
    connect(size_combo, SIGNAL(currentIndexChanged(int)), 
	    this, SLOT(sizeComboAdjust(int)));
    size_combo->insertItem(0,"Extents");
    size_combo->insertItem(1,"MB");
    size_combo->insertItem(2,"GB");
    size_combo->insertItem(3,"TB");
    size_combo->setInsertPolicy(QComboBox::NoInsert);
    size_combo->setCurrentIndex(0);

    size_edit->setText( QString("%1").arg( current_size / vg->getExtentSize() ));
    if(current_size / (1024 * 1024) > (1024 * 1024))
	size_combo->setCurrentIndex(3);
    else if(current_size / (1024 * 1024) > 1024 )
	size_combo->setCurrentIndex(2);
    else
	size_combo->setCurrentIndex(1);
    
    size_combo_last_index = size_combo->currentIndex();
    size_layout->addWidget(size_edit);
    size_layout->addWidget(size_combo);
    layout->addLayout(size_layout);
}

QStringList LVReduceDialog::argumentsFS()
{
    long long suffix;
    double temp;
    QStringList fs_arguments;

    if(size_combo->currentIndex()){
	if (size_combo->currentIndex() == 3)       // Combobox = Terabytes
	    suffix = (long long)(1024 * 1024) * (1024 * 1024);
	if (size_combo->currentIndex() == 2)       // Combobox = Gigabytes
	    suffix = 1024 * 1024 * 1024;
	if (size_combo->currentIndex() == 1)       // Combobox = Megabytes
	    suffix = 1024 * 1024;
	temp = (size_edit->text()).toDouble();      
	new_extents = qRound64(( temp * (suffix /(double) (vg->getExtentSize())) ));
    }
    else
	new_extents = qRound64((size_edit->text()).toDouble());
    if (new_extents <= 0)
        new_extents = 1;
    new_size = (vg->getExtentSize()) * new_extents;

    if( (lv->getFilesystem() == "ext2") || (lv->getFilesystem() == "ext3") ){
	fs_arguments << "resize2fs" << "-f"
		     << "/dev/mapper/" + vg->getName() + "-" + lv->getName() 
		     << QString("%1k").arg(new_size / 1024);
    }
    return fs_arguments;
}

QStringList LVReduceDialog::argumentsLV()
{
    long long suffix;
    double temp;
    QStringList lv_arguments;

    if(size_combo->currentIndex()){
	if (size_combo->currentIndex() == 3)       // Combobox = Terabytes
	    suffix = (long long)(1024 * 1024) * (1024 * 1024);
	if (size_combo->currentIndex() == 2)       // Combobox = Gigabytes
	    suffix = 1024 * 1024 * 1024;
	if (size_combo->currentIndex() == 1)       // Combobox = Megabytes
	    suffix = 1024 * 1024;
	temp = (size_edit->text()).toDouble();      
	new_extents = qRound64(( temp * (suffix /(double) (vg->getExtentSize())) ));
    }
    else
	new_extents = qRound64((size_edit->text()).toDouble());
    if (new_extents <= 0)
        new_extents = 1;
    new_size = (vg->getExtentSize()) * new_extents;

    lv_arguments << "lvreduce" << "--force" 
		 << "--extents" << QString("%1").arg(new_extents)
		 << vg->getName() + "/" + lv->getName();
    
    return lv_arguments;
}

void LVReduceDialog::validateInput(QString text)
{
    int pos = 0;
    if(size_validator->validate(text , pos) == QValidator::Acceptable)
	enableButtonOk(TRUE);
    else
	enableButtonOk(FALSE);
}

void LVReduceDialog::sizeComboAdjust(int index)
{
    long long es;
    unsigned long long exts;
    long double sized;

    es = vg->getExtentSize();
    if(size_combo_last_index){
	sized = (size_edit->displayText()).toDouble();
	if(size_combo_last_index == 1)
	    sized *= (long double)0x100000;
	if(size_combo_last_index == 2)
	    sized *= (long double)0x40000000;
	if(size_combo_last_index == 3){
	    sized *= (long double)(0x100000);
	    sized *= (long double)(0x100000);
	}
	sized /= es;
	if (sized < 0)
	    sized = 0;
	exts = qRound64(sized);
    }
    else
	exts =  (size_edit->displayText()).toULongLong();
    if(index){
	sized = (long double)exts * es;
	if(index == 1){
	    size_validator->setTop(current_size / (long double)0x100000);
	    sized /= (long double)0x100000;
	}
	if(index == 2){
	    size_validator->setTop(current_size / (long double)0x40000000);
	    sized /= (long double)0x40000000; 
	}
	if(index == 3){
	    size_validator->setTop(current_size / ((long double)0x100000 * 0x100000));
	    sized /= (long double)(0x100000);
	    sized /= (long double)(0x100000);
	}
	size_edit->setText(QString("%1").arg((double)sized));
    }
    else{
	size_validator->setTop(current_size / es);
	size_edit->setText(QString("%1").arg(exts));
    }
    size_combo_last_index = index;
}
