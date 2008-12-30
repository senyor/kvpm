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
#include <KLocale>
#include <QtGui>

#include "logvol.h"
#include "lvreduce.h"
#include "processprogress.h"
#include "sizetostring.h"
#include "volgroup.h"


bool lv_reduce(LogVol *logicalVolume)
{
    QString fs, state;
    QStringList fs_args, lv_args;
    
    fs    = logicalVolume->getFilesystem();
    state = logicalVolume->getState();

    QString warning_message = i18n("Currently only the ext2 and ext3  file systems "
				   "are supported for file system reduction. If this " 
				   "logical volume is reduced any data it contains "
				   "will be lost!");

    QString warning_message2 = i18n("If this <b>Inactive</b> logical volume is reduced "
				    "any data it contains will be lost!");

    if( state != "Active" ){
	if(KMessageBox::warningContinueCancel(0, warning_message2) != KMessageBox::Continue)
	    return false;
	else{
	    LVReduceDialog dialog(logicalVolume);
	    dialog.exec();
	    if(dialog.result() == QDialog::Accepted){
	        ProcessProgress reduce_lv(dialog.argumentsLV(), i18n("Reducing volume..."), true);
		return true;
	    }
	    else
		return false;
	}
    }
    else if( logicalVolume->isMirror() ){
        KMessageBox::error(0, i18n("Resizing mirrors is not supported yet") );
	return false;
    }
    else if( logicalVolume->isSnap() ){
	LVReduceDialog dialog(logicalVolume);
	dialog.exec();
	if(dialog.result() == QDialog::Accepted){
	    ProcessProgress reduce_lv(dialog.argumentsLV(), 
				      i18n("Reducing volume..."), 
				      true);
	    return true;
	}
	return true;
    }
    else if( logicalVolume->isOrigin() ){
        KMessageBox::error(0, i18n("Resizing snapshot origin is not supported yet") );
	return false;
    }
    else if( logicalVolume->isMounted() ){
        KMessageBox::error(0, i18n("The filesystem must be unmounted first") );
	return false;
    }
    else if( (fs != "ext2") && (fs != "ext3") ){
	if(KMessageBox::warningContinueCancel(0, warning_message) != KMessageBox::Continue)
	    return false;
	else{
	    LVReduceDialog dialog(logicalVolume);
	    dialog.exec();
	    if(dialog.result() == QDialog::Accepted){
	        ProcessProgress reduce_lv(dialog.argumentsLV(), i18n("Reducing volume..."), true);
		return true;
	    }
	    else
		return false;
	}
    }
    else{
	LVReduceDialog dialog(logicalVolume);
	dialog.exec();
	if(dialog.result() == QDialog::Accepted){
	    ProcessProgress reduce_fs(dialog.argumentsFS(), i18n("Reducing filesystem..."), true);
	    if( !reduce_fs.exitCode() )
	        ProcessProgress reduce_lv(dialog.argumentsLV(), i18n("Reducing volume..."), true);
	    return true;
	}
	else
	    return false;
    }
}


LVReduceDialog::LVReduceDialog(LogVol *logicalVolume, QWidget *parent) : 
    KDialog(parent),
    m_lv(logicalVolume)
{

    m_vg = m_lv->getVolumeGroup();
    setWindowTitle( i18n("Reduce Logical Volume") );

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    m_current_lv_size = m_lv->getSize();

    QLabel *ext_size_label = new QLabel( i18n("Extent size: %1").arg(sizeToString(m_vg->getExtentSize())) );

    QLabel *current_lv_size_label = new QLabel( i18n("Current size: %1").arg(sizeToString(m_current_lv_size)) );

    layout->addWidget(ext_size_label);
    layout->addWidget(current_lv_size_label);
    QHBoxLayout *size_layout = new QHBoxLayout();
    size_layout->addWidget(new QLabel( i18n("New size: ") ));
    
    m_size_edit = new KLineEdit();
    m_size_validator = new KDoubleValidator(m_size_edit);
    m_size_validator->setBottom(0.0);
    m_size_edit->setValidator(m_size_validator);

    connect(m_size_edit, SIGNAL(textEdited(QString)),
	    this, SLOT(validateInput(QString)));
    
    m_size_combo = new KComboBox();

    connect(m_size_combo, SIGNAL(currentIndexChanged(int)), 
	    this, SLOT(sizeComboAdjust(int)));

    m_size_combo->insertItem(0,"Extents");
    m_size_combo->insertItem(1,"MB");
    m_size_combo->insertItem(2,"GB");
    m_size_combo->insertItem(3,"TB");
    m_size_combo->setInsertPolicy(QComboBox::NoInsert);
    m_size_combo->setCurrentIndex(0);

    m_size_edit->setText( QString("%1").arg( m_current_lv_size / m_vg->getExtentSize() ));

    if(m_current_lv_size / (1024 * 1024) > (1024 * 1024))
	m_size_combo->setCurrentIndex(3);
    else if(m_current_lv_size / (1024 * 1024) > 1024 )
	m_size_combo->setCurrentIndex(2);
    else
	m_size_combo->setCurrentIndex(1);
    
    m_size_combo_last_index = m_size_combo->currentIndex();
    size_layout->addWidget(m_size_edit);
    size_layout->addWidget(m_size_combo);
    layout->addLayout(size_layout);
}

QStringList LVReduceDialog::argumentsFS()
{
    QStringList fs_arguments;

    long long new_size = getSizeEditExtents( m_size_combo->currentIndex() );
    new_size *= m_vg->getExtentSize();

    if( (m_lv->getFilesystem() == "ext2") || (m_lv->getFilesystem() == "ext3") ){
	fs_arguments << "resize2fs" 
		     << "-f"
		     << "/dev/mapper/" + m_vg->getName() + "-" + m_lv->getName() 
		     << QString("%1k").arg(new_size / 1024);
    }

    return fs_arguments;
}

QStringList LVReduceDialog::argumentsLV()
{
    QStringList lv_arguments;
    long long new_extents;
    
    new_extents = getSizeEditExtents( m_size_combo->currentIndex() );

    lv_arguments << "lvreduce" 
		 << "--force" 
		 << "--extents" 
		 << QString("%1").arg(new_extents)
		 << m_vg->getName() + "/" + m_lv->getName();
    
    return lv_arguments;
}

void LVReduceDialog::validateInput(QString text)
{
    int pos = 0;

    if(m_size_validator->validate(text , pos) == QValidator::Acceptable)
	enableButtonOk(true);
    else
	enableButtonOk(false);
}

void LVReduceDialog::sizeComboAdjust(int index)
{
    long long extent_size;
    unsigned long long extents;
    long double sized;

    extent_size = m_vg->getExtentSize();
    extents     = getSizeEditExtents(m_size_combo_last_index);

    if(index){
	sized = (long double)extents * extent_size;
	if(index == 1){
	    m_size_validator->setTop(m_current_lv_size / (long double)0x100000);
	    sized /= (long double)0x100000;
	}
	if(index == 2){
	    m_size_validator->setTop(m_current_lv_size / (long double)0x40000000);
	    sized /= (long double)0x40000000; 
	}
	if(index == 3){
	    m_size_validator->setTop(m_current_lv_size / ((long double)0x100000 * 0x100000));
	    sized /= (long double)(0x100000);
	    sized /= (long double)(0x100000);
	}
	m_size_edit->setText(QString("%1").arg((double)sized));
    }
    else{
	m_size_validator->setTop(m_current_lv_size / extent_size);
	m_size_edit->setText(QString("%1").arg(extents));
    }

    m_size_combo_last_index = index;
}

long long LVReduceDialog::getSizeEditExtents(int index)
{
    long long extent_size = m_vg->getExtentSize();
    long long extents;
    long double sized;

    if(index){  
	sized = (m_size_edit->displayText()).toDouble();

	if(m_size_combo_last_index == 1){      // combo index = Megabytes
	    sized *= (long double)0x100000;
	}
	else if(m_size_combo_last_index == 2){ // combo index = Gigabytes
	    sized *= (long double)0x40000000;
	}
	else{                                  // combo index = Terabytes
	    sized *= (long double)(0x100000);
	    sized *= (long double)(0x100000);
	}

	sized /= extent_size;

	if (sized < 0)
	    sized = 0;

	extents = qRound64(sized);
    }
    else                                       // combo index = Extents
	extents =  ( m_size_edit->displayText() ).toLongLong();

    return extents;
}
