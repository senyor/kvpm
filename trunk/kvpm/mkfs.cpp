/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <KMessageBox>
#include <KDialog>
#include <KProgressDialog>
#include <KLocale>
#include <QtGui>

#include "logvol.h"
#include "mkfs.h"
#include "processprogress.h"
#include "storagedevice.h"
#include "storagepartition.h"
#include "volgroup.h"

bool call_dialog(QString devicePath);


bool make_fs(LogVol *logicalVolume)
{
    if( logicalVolume ){
	
	QString full_name = logicalVolume->getFullName();
    
	QString error_message = i18n("The volume: <b>%1</b> is mounted. It must be "
				     "unmounted before a new filesystem " 
				     "can be written on it").arg(full_name);

	if( logicalVolume->isMounted() ){
	    KMessageBox::error(0, error_message);
	    return false;
	}
	else
	    return call_dialog("/dev/" + full_name );
    }
    else{
	qDebug() << "make_fs was passed a NULL Logical volume pointer!!";
	return false;
    }
}

bool make_fs(StoragePartition *partition)
{
    if( partition ){

	QString full_name = partition->getPartitionPath();
    
	QString error_message = i18n("The partition: <b>%1</b> is mounted. It must "
				     "be unmounted before a new filesystem " 
				     "can be written on it").arg(full_name);

	if( partition->isMounted() ){
	    KMessageBox::error(0, error_message);
	    return false;
	}
	else
	    return call_dialog( full_name );
    }
    else{
	qDebug() << "make_fs was passed a NULL StoragePartition pointer!!";
	return false;
    }
}

bool call_dialog(QString devicePath)
{
    QString warning_message = i18n("By writing a new file system to this device or volume "
				   "any existing data on it will be lost.");

    if(KMessageBox::warningContinueCancel(0, warning_message) != KMessageBox::Continue){
	return false;
    }
    else{
	MkfsDialog dialog(devicePath);
	dialog.exec();
	if(dialog.result() == QDialog::Accepted){
	    ProcessProgress mkfs(dialog.arguments(), i18n("Writing filesystem..."), true);
	    return true;
	}
	else{
	    return false;
	}
    }
}

MkfsDialog::MkfsDialog(QString devicePath, QWidget *parent) : 
    KDialog(parent),
    m_path(devicePath)
{

    QWidget *dialog_body = new QWidget();
    setMainWidget(dialog_body);
    
    QVBoxLayout *layout = new QVBoxLayout;
    QHBoxLayout *radio_layout = new QHBoxLayout;
    QVBoxLayout *radio_left_layout   = new QVBoxLayout;
    QVBoxLayout *radio_center_layout = new QVBoxLayout;
    QVBoxLayout *radio_right_layout  = new QVBoxLayout;
    radio_layout->addLayout(radio_left_layout);
    radio_layout->addLayout(radio_center_layout);
    radio_layout->addLayout(radio_right_layout);
    dialog_body->setLayout(layout);

    QLabel *label = new QLabel( i18n("Device") );
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    label = new QLabel(m_path);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    radio_box = new QGroupBox( i18n("Filesystem") );
    ext2    = new QRadioButton("Ext2", this);
    ext3    = new QRadioButton("Ext3", this);
    ext4    = new QRadioButton("Ext4", this);
    reiser  = new QRadioButton("Reiser", this);
    reiser4 = new QRadioButton("Reiser4", this);
    jfs     = new QRadioButton("jfs", this);
    xfs     = new QRadioButton("xfs", this);
    swap    = new QRadioButton( i18n("Linux swap"), this);
    vfat    = new QRadioButton("ms-dos", this);
    radio_left_layout->addWidget(ext2);
    radio_left_layout->addWidget(ext3);
    radio_left_layout->addWidget(ext4);
    radio_center_layout->addWidget(reiser);
    radio_center_layout->addWidget(reiser4);
    radio_center_layout->addWidget(swap);
    radio_right_layout->addWidget(jfs);
    radio_right_layout->addWidget(xfs);
    radio_right_layout->addWidget(vfat);
    ext3->setChecked(true);
    
    radio_box->setLayout(radio_layout);
    layout->addWidget(radio_box);
}

QStringList MkfsDialog::arguments()
{
    QStringList arguments;
    QStringList mkfs_options;
    QString type;
    
    if(ext2->isChecked()){
	type = "ext2";
    }
    else if(ext3->isChecked()){
	type = "ext3";
    }
    else if(ext4->isChecked()){
	type = "ext4";
    }
    else if(reiser->isChecked()){
	mkfs_options << "-q";
	type = "reiserfs";
    }
    else if(reiser4->isChecked()){
	mkfs_options << "-y";
	type = "reiser4";
    }
    else if(jfs->isChecked()){
	mkfs_options << "-q";
	type = "jfs";
    }
    else if(xfs->isChecked()){
	mkfs_options << "-q" << "-f";
	type = "xfs";
    }
    else if(swap->isChecked()){
	type = "swap";
    }
    else if(vfat->isChecked()){
	type = "vfat";
    }
    else{
	type = "ext3";
	qDebug() << "Reached the default else in mkfs.cpp. How did that happen?";
    }
    
    if(type != "swap"){

	arguments << "mkfs";

	if(mkfs_options.size()){
	    arguments << "-t" 
		      << type 
		      << mkfs_options 
		      << m_path;
	}
	else{
	    arguments << "-t" 
		      << type 
		      << m_path;
	}
    }
    else{
	arguments << "mkswap" 
		  << m_path;
    }
    
    return arguments;
}

