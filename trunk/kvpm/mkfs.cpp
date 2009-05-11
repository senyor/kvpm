/*
 *
 * 
 * Copyright (C) 2008, 2009 Benjamin Scott   <benscott@nwlink.com>
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


bool make_fs(LogVol *logicalVolume)
{

    QString warning_message = i18n("By writing a new file system to this volume "
				   "any existing data on it will be lost.");

    QString error_message = i18n("The volume: <b>%1</b> is mounted. It must be "
                                 "unmounted before a new filesystem " 
                                 "can be written on it").arg( logicalVolume->getFullName() );

    if( logicalVolume->isMounted() ){
        KMessageBox::error(0, error_message);
        return false;
    }
    else{
        if(KMessageBox::warningContinueCancel(0, warning_message) == KMessageBox::Continue){

            MkfsDialog dialog(logicalVolume);
            dialog.exec();
            if(dialog.result() == QDialog::Accepted){
                ProcessProgress mkfs(dialog.arguments(), i18n("Writing filesystem..."), true);
                return true;
            }
        }
    }

    return false;
}


bool make_fs(StoragePartition *partition)
{

    QString warning_message = i18n("By writing a new file system to this device "
				   "any existing data on it will be lost.");
    
    QString error_message = i18n("The partition: <b>%1</b> is mounted. It must "
                                 "be unmounted before a new filesystem " 
                                 "can be written on it").arg( partition->getPartitionPath() );

    if( partition->isMounted() ){
        KMessageBox::error(0, error_message);
        return false;
    }
    else{
        if(KMessageBox::warningContinueCancel(0, warning_message) == KMessageBox::Continue){

            MkfsDialog dialog(partition);
            dialog.exec();
            if(dialog.result() == QDialog::Accepted){
                ProcessProgress mkfs(dialog.arguments(), i18n("Writing filesystem..."), true);
                return true;
            }
        }
    }

    return false;
}


MkfsDialog::MkfsDialog(LogVol *logicalVolume, QWidget *parent) : KDialog(parent)
{
    m_path = QString( "/dev/" + logicalVolume->getFullName() );

    m_stride_size = logicalVolume->getSegmentStripeSize( 0 );   // 0 if not striped
    m_stride_count = logicalVolume->getSegmentStripes( 0 );

    buildDialog();
}


MkfsDialog::MkfsDialog(StoragePartition *partition, QWidget *parent) : KDialog(parent)
{
    m_path = partition->getPartitionPath();
    m_stride_size = 0;
    m_stride_count = 1;

    buildDialog();
}


void MkfsDialog::buildDialog()
{

    m_tab_widget = new KTabWidget(this);

    QWidget *dialog_body = new QWidget();
    QWidget *advanced_options = new QWidget();
    m_tab_widget->addTab(dialog_body, i18n("General") );
    m_tab_widget->addTab(advanced_options, i18n("Advanced") );

    setMainWidget(m_tab_widget);
    setCaption( i18n("Write filesystem") );

    QVBoxLayout *layout = new QVBoxLayout;
    QHBoxLayout *radio_layout = new QHBoxLayout;
    QVBoxLayout *radio_left_layout   = new QVBoxLayout;
    QVBoxLayout *radio_center_layout = new QVBoxLayout;
    QVBoxLayout *radio_right_layout  = new QVBoxLayout;
    radio_layout->addLayout(radio_left_layout);
    radio_layout->addLayout(radio_center_layout);
    radio_layout->addLayout(radio_right_layout);
    dialog_body->setLayout(layout);

    QLabel *label = new QLabel( i18n("Write filesystem on:") );
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

    QVBoxLayout *advanced_layout = new QVBoxLayout;
    QHBoxLayout *block_layout  = new QHBoxLayout;
    QHBoxLayout *name_layout   = new QHBoxLayout;
    QHBoxLayout *stride_layout = new QHBoxLayout;
    QHBoxLayout *count_layout  = new QHBoxLayout;
    advanced_options->setLayout(advanced_layout);

    block_layout->addWidget( new QLabel( i18n("Block size: ") ) );
    m_block_combo = new KComboBox();
    m_block_combo->insertItem(0, i18n("automatic") );
    m_block_combo->insertItem(1,"1024 KiB");
    m_block_combo->insertItem(2,"2048 KiB");
    m_block_combo->insertItem(3,"4096 KiB");
    m_block_combo->setInsertPolicy(KComboBox::NoInsert);
    m_block_combo->setCurrentIndex(0);
    block_layout->addWidget(m_block_combo);
    block_layout->addStretch();
    advanced_layout->addLayout(block_layout);

    name_layout->addWidget( new QLabel( i18n("Volume name: ") ) );
    m_volume_edit = new KLineEdit();
    name_layout->addWidget(m_volume_edit);
    name_layout->addStretch();
    advanced_layout->addLayout(name_layout);

    m_stripe_box = new QGroupBox( i18n("Striping") );
    m_stripe_box->setCheckable(true);
    m_stripe_box->setChecked(false);
    QVBoxLayout *stripe_layout = new QVBoxLayout();
    m_stripe_box->setLayout(stripe_layout);
    advanced_layout->addWidget(m_stripe_box);

    stride_layout->addWidget( new QLabel( i18n("Stride size: ") ) );
    m_stride_edit = new KLineEdit( QString("%1").arg(m_stride_size) );
    QIntValidator *stride_validator = new QIntValidator(m_stride_edit);
    m_stride_edit->setValidator(stride_validator);
    stride_validator->setBottom(512);
    stride_layout->addWidget(m_stride_edit);
    stride_layout->addStretch();
    stripe_layout->addLayout(stride_layout);

    count_layout->addWidget( new QLabel( i18n("Strides per stripe: ") ) );
    m_count_edit = new KLineEdit( QString("%1").arg(m_stride_count) );
    QIntValidator *count_validator = new QIntValidator(m_count_edit);
    m_count_edit->setValidator(count_validator);
    count_validator->setBottom(1);
    count_layout->addWidget(m_count_edit);
    count_layout->addStretch();
    stripe_layout->addLayout(count_layout);


    connect(ext2,  SIGNAL(toggled(bool)), 
            this, SLOT(setAdvancedTab(bool)));

    connect(ext3,  SIGNAL(toggled(bool)), 
            this, SLOT(setAdvancedTab(bool)));

    connect(ext4,  SIGNAL(toggled(bool)), 
            this, SLOT(setAdvancedTab(bool)));

}

void MkfsDialog::setAdvancedTab(bool)
{
    if( ext2->isChecked() || ext3->isChecked() || ext4->isChecked() )
        m_tab_widget->setTabEnabled( 1, true );
    else
        m_tab_widget->setTabEnabled( 1, false );
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

