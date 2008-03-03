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
#include <KDialog>
#include <KProgressDialog>

#include <QtGui>

#include "logvol.h"
#include "mkfs.h"
#include "processprogress.h"
#include "storagedevice.h"
#include "storagepartition.h"
#include "volgroup.h"

bool call_dialog(QString DevicePath);


bool make_fs(LogVol *LogicalVolume)
{
    if( LogicalVolume ){
	
	QString full_name = LogicalVolume->getFullName();
    
	QString error_message = "The volume: <b>" + full_name + "</b> is mounted. It must be ";
	error_message.append("unmounted before a new filesystem can be written on it");

	if( LogicalVolume->isMounted() ){
	    KMessageBox::error(0, error_message);
	    return FALSE;
	}
	else
	    return call_dialog("/dev/" + full_name );
    }
    else{
	qDebug() << "make_fs was passed a NULL Logical volume pointer!!";
	return FALSE;
    }
}

bool make_fs(StoragePartition *Partition)
{
    if( Partition ){

	QString full_name = Partition->getPartitionPath();
    
	QString error_message = "The partition: <b>" + full_name + "</b> is mounted. It must ";
	error_message.append("be unmounted before a new filesystem can be written on it");

	if( Partition->isMounted() ){
	    KMessageBox::error(0, error_message);
	    return FALSE;
	}
	else
	    return call_dialog( full_name );
    }
    else{
	qDebug() << "make_fs was passed a NULL StoragePartition pointer!!";
	return FALSE;
    }
}

bool call_dialog(QString DevicePath)
{
    QString warning_message = "By writing a new file system to this device or volume";
    warning_message.append(" any existing data on it will be lost. ");

    if(KMessageBox::warningContinueCancel(0, warning_message) != KMessageBox::Continue)
	return FALSE;
    else{
	MkfsDialog dialog(DevicePath);
	dialog.exec();
	if(dialog.result() == QDialog::Accepted){
	    ProcessProgress mkfs(dialog.arguments(), "Writing filesystem...", TRUE);
	    return TRUE;
	}
	else
	    return FALSE;
    }
}

MkfsDialog::MkfsDialog(QString DevicePath, QWidget *parent) : KDialog(parent)
{
    QWidget *dialog_body = new QWidget();
    setMainWidget(dialog_body);

    path = DevicePath;
    
    QVBoxLayout *layout = new QVBoxLayout;
    QHBoxLayout *radio_layout = new QHBoxLayout;
    QVBoxLayout *radio_left_layout = new QVBoxLayout;
    QVBoxLayout *radio_right_layout = new QVBoxLayout;
    radio_layout->addLayout(radio_left_layout);
    radio_layout->addLayout(radio_right_layout);
    dialog_body->setLayout(layout);

    QLabel *label = new QLabel("Device");
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    label = new QLabel(path);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    radio_box = new QGroupBox("Filesystem");
    ext2 = new QRadioButton("Ext2", this);
    ext3 = new QRadioButton("Ext3", this);
    reiser = new QRadioButton("Reiser", this);
    jfs = new QRadioButton("jfs", this);
    xfs = new QRadioButton("xfs", this);
    swap = new QRadioButton("Linux swap", this);
    vfat = new QRadioButton("ms-dos", this);
    radio_left_layout->addWidget(ext2);
    radio_left_layout->addWidget(ext3);
    radio_left_layout->addWidget(reiser);
    radio_left_layout->addWidget(jfs);
    radio_right_layout->addWidget(xfs);
    radio_right_layout->addWidget(vfat);
    radio_right_layout->addWidget(swap);
    ext3->setChecked(TRUE);
    
    radio_box->setLayout(radio_layout);
    layout->addWidget(radio_box);
}

QStringList MkfsDialog::arguments()
{
    QStringList arguments;
    QStringList mkfs_options;
    
    if(ext2->isChecked())    
	type = "ext2";
    if(ext3->isChecked())
	type = "ext3";
    if(reiser->isChecked()){
	mkfs_options << "-q";
	type = "reiserfs";
    }
    if(jfs->isChecked()){
	mkfs_options << "-q";
	type = "jfs";
    }
    
    if(xfs->isChecked()){
	mkfs_options << "-q" << "-f";
	type = "xfs";
    }
    if(swap->isChecked())
	type = "swap";
    
    if(vfat->isChecked())
	type = "vfat";
     
    if(type != "swap"){
	arguments << "mkfs";
	if(mkfs_options.size())    
	    arguments << "-t" << type << mkfs_options << path;
	else
	    arguments << "-t" << type << path;
    }
    else
	arguments << "mkswap" << path;
    return arguments;
}

