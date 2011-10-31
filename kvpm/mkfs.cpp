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

#include "mkfs.h"

#include <KMessageBox>
#include <KDialog>
#include <KProgressDialog>
#include <KLocale>
#include <QtGui>

#include "logvol.h"
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
                                 "can be written on it", logicalVolume->getFullName() );

    QByteArray zero_array(128 * 1024, '\0');

    if( logicalVolume->isMounted() ){
        KMessageBox::error(0, error_message);
        return false;
    }
    else{
        if(KMessageBox::warningContinueCancel(0, warning_message) == KMessageBox::Continue){

            MkfsDialog dialog(logicalVolume);
            dialog.exec();
            if(dialog.result() == QDialog::Accepted){
                ProcessProgress mkfs( dialog.arguments() );
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
                                 "can be written on it", partition->getName() );

    if( partition->isMounted() ){
        KMessageBox::error(0, error_message);
        return false;
    }
    else{
        if(KMessageBox::warningContinueCancel(0, warning_message) == KMessageBox::Continue){

            MkfsDialog dialog(partition);
            dialog.exec();
            if(dialog.result() == QDialog::Accepted){
                ProcessProgress mkfs( dialog.arguments() );
                return true;
            }
        }
    }
    return false;
}

MkfsDialog::MkfsDialog(LogVol *logicalVolume, QWidget *parent) : KDialog(parent)
{
    m_path = logicalVolume->getMapperPath();

    m_stride_size = 1; // logicalVolume->getSegmentStripeSize( 0 ); <-- must convert to blocks!
    m_stride_count = logicalVolume->getSegmentStripes( 0 );

    QWidget *dialog_body = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    dialog_body->setLayout(layout);

    QLabel *label = new QLabel( i18n("<b>Write filesystem on: %1</b>", m_path) );
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    m_tab_widget = new KTabWidget(this);
    m_tab_widget->addTab(generalTab(), i18n("Filesystem type") );
    m_tab_widget->addTab(advancedTab(), i18nc("Less used, dangerous or complex options", "Advanced options") );
    layout->addWidget(m_tab_widget);

    setMainWidget(dialog_body);
    setCaption( i18n("Write filesystem") );

    setAdvancedTab(true);

    connect(this, SIGNAL(okClicked()), 
            this, SLOT(clobberFS()));
}

MkfsDialog::MkfsDialog(StoragePartition *partition, QWidget *parent) : KDialog(parent)
{
    m_path = partition->getName();
    m_stride_size = 1;
    m_stride_count = 1;

    QWidget *dialog_body = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    dialog_body->setLayout(layout);

    QLabel *label = new QLabel( i18n("<b>Write filesystem on: %1</b>", m_path) );
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    m_tab_widget = new KTabWidget(this);
    m_tab_widget->addTab(generalTab(), i18n("Filesystem type") );
    m_tab_widget->addTab(advancedTab(), i18nc("Less used, dangerous or complex options", "Advanced options") );
    layout->addWidget(m_tab_widget);

    setMainWidget(dialog_body);
    setCaption( i18n("Write filesystem") );

    setAdvancedTab(true);

    connect(this, SIGNAL(okClicked()), 
            this, SLOT(clobberFS()));
}

QWidget* MkfsDialog::generalTab()
{
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();
    tab->setLayout(layout);

    radio_box = new QGroupBox( i18n("Filesystem") );
    QGridLayout *radio_layout = new QGridLayout;
    radio_box->setLayout(radio_layout);
    layout->addWidget(radio_box);
    ext2    = new QRadioButton("ext2", this);
    ext3    = new QRadioButton("ext3", this);
    ext4    = new QRadioButton("ext4", this);
    btrfs   = new QRadioButton("btrfs", this);
    ntfs    = new QRadioButton("ntfs", this);
    reiser  = new QRadioButton("reiser", this);
    reiser4 = new QRadioButton("reiser4", this);
    jfs     = new QRadioButton("jfs", this);
    xfs     = new QRadioButton("xfs", this);
    swap    = new QRadioButton( i18n("Linux swap"), this);
    vfat    = new QRadioButton("ms-dos", this);
    m_clobber_fs_check = new QCheckBox("Remove old filesystem first", this);
    m_clobber_fs_check->setChecked(true);
    radio_layout->addWidget(ext2, 1, 0);
    radio_layout->addWidget(ext3, 2, 0);
    radio_layout->addWidget(ext4, 3, 0);
    radio_layout->addWidget(btrfs, 4, 0);
    radio_layout->addWidget(ntfs, 4, 1);
    radio_layout->addWidget(reiser, 1, 1);
    radio_layout->addWidget(reiser4, 2, 1);
    radio_layout->addWidget(swap, 3, 1);
    radio_layout->addWidget(jfs, 1, 2);
    radio_layout->addWidget(xfs, 2, 2);
    radio_layout->addWidget(vfat, 3, 2);
    radio_layout->setRowStretch(5, 1);
    radio_layout->addWidget(m_clobber_fs_check, 6, 0, 1, -1, Qt::AlignLeft);
    ext4->setChecked(true);
    layout->addStretch();

    connect(ext2,  SIGNAL(toggled(bool)), 
            this, SLOT(setAdvancedTab(bool)));

    connect(ext3,  SIGNAL(toggled(bool)), 
            this, SLOT(setAdvancedTab(bool)));

    connect(ext4,  SIGNAL(toggled(bool)), 
            this, SLOT(setAdvancedTab(bool)));

    return tab;
}

QWidget* MkfsDialog::advancedTab()
{
    QWidget *tab = new QWidget();

    QVBoxLayout *layout = new QVBoxLayout();
    tab->setLayout(layout);

    QLabel *override_label = new QLabel( i18n("<b>Override mkfs defaults</b>") );
    override_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(override_label);

    QHBoxLayout *lower_layout = new QHBoxLayout();
    layout->addLayout(lower_layout);

    QVBoxLayout *left_layout = new QVBoxLayout;
    QVBoxLayout *right_layout = new QVBoxLayout;
    lower_layout->addLayout(left_layout);
    lower_layout->addLayout(right_layout);

    QHBoxLayout *reserved_layout = new QHBoxLayout;
    QHBoxLayout *block_layout  = new QHBoxLayout;
    QHBoxLayout *isize_layout  = new QHBoxLayout;
    QHBoxLayout *name_layout   = new QHBoxLayout;
    QHBoxLayout *stride_layout = new QHBoxLayout;
    QHBoxLayout *count_layout  = new QHBoxLayout;
    QVBoxLayout *misc_layout   = new QVBoxLayout;
    QHBoxLayout *inode_layout  = new QHBoxLayout;
    QHBoxLayout *total_layout  = new QHBoxLayout;

    m_misc_box = new QGroupBox();
    m_misc_box->setLayout(misc_layout);
    left_layout->addWidget(m_misc_box);

    reserved_layout->addWidget( new QLabel( i18n("Reserved space: ") ) );
    m_reserved_spin = new QSpinBox;
    m_reserved_spin->setRange(0, 100);
    m_reserved_spin->setValue(5);
    m_reserved_spin->setPrefix("%");
    reserved_layout->addWidget(m_reserved_spin);
    reserved_layout->addStretch();
    misc_layout->addLayout(reserved_layout);

    block_layout->addWidget( new QLabel( i18n("Block size: ") ) );
    m_block_combo = new KComboBox();
    m_block_combo->insertItem(0, i18nc("Let the program decide", "default") );
    m_block_combo->insertItem(1,"1024 KiB");
    m_block_combo->insertItem(2,"2048 KiB");
    m_block_combo->insertItem(3,"4096 KiB");
    m_block_combo->setInsertPolicy(KComboBox::NoInsert);
    m_block_combo->setCurrentIndex(0);
    block_layout->addWidget(m_block_combo);
    block_layout->addStretch();
    misc_layout->addLayout(block_layout);

    isize_layout->addWidget( new QLabel( i18n("Inode size: ") ) );
    m_inode_combo = new KComboBox();
    m_inode_combo->insertItem(0, i18nc("Let the program decide", "default") );
    m_inode_combo->insertItem(1,"128 Bytes");
    m_inode_combo->insertItem(2,"256 Bytes");
    m_inode_combo->insertItem(3,"512 Bytes");
    m_inode_combo->setInsertPolicy(KComboBox::NoInsert);
    m_inode_combo->setCurrentIndex(0);
    isize_layout->addWidget(m_inode_combo);
    isize_layout->addStretch();
    misc_layout->addLayout(isize_layout);

    name_layout->addWidget( new QLabel( i18n("Name or label: ") ) );
    m_volume_edit = new KLineEdit();
    name_layout->addWidget(m_volume_edit);
    name_layout->addStretch();
    misc_layout->addLayout(name_layout);

    inode_layout->addWidget( new QLabel( i18n("Bytes / inode: ") ) );
    m_inode_edit = new KLineEdit();
    QIntValidator *inode_validator = new QIntValidator(m_inode_edit);
    m_inode_edit->setValidator(inode_validator);
    inode_validator->setBottom(0);
    inode_layout->addWidget(m_inode_edit);
    misc_layout->addLayout(inode_layout);

    total_layout->addWidget( new QLabel( i18n("Total inodes: ") ) );
    m_total_edit = new KLineEdit();
    QIntValidator *total_validator = new QIntValidator(m_total_edit);
    m_total_edit->setValidator(total_validator);
    total_validator->setBottom(0);
    total_layout->addWidget(m_total_edit);
    misc_layout->addLayout(total_layout);

    m_stripe_box = new QGroupBox( i18n("Striping") );
    m_stripe_box->setCheckable(true);
    m_stripe_box->setChecked(false);
    QVBoxLayout *stripe_layout = new QVBoxLayout();
    m_stripe_box->setLayout(stripe_layout);
    right_layout->addWidget(m_stripe_box);

    stride_layout->addWidget( new QLabel( i18n("Stride size in blocks: ") ) );
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

    m_base_options_box = new QGroupBox("Basic options");
    m_ext4_options_box = new QGroupBox("Ext4 options");
    m_base_options_box->setCheckable(true);
    m_ext4_options_box->setCheckable(true);
    m_base_options_box->setChecked(false);
    m_ext4_options_box->setChecked(false);

    QVBoxLayout *base_options_layout = new QVBoxLayout();
    QVBoxLayout *ext4_options_layout = new QVBoxLayout();

    m_ext_attr_check     = new QCheckBox( i18n("Extended attributes") );
    m_resize_inode_check = new QCheckBox( i18n("Resize inode") );
    m_resize_inode_check->setEnabled(false);
    m_dir_index_check    = new QCheckBox( i18n("Directory B-Tree index") );
    m_filetype_check     = new QCheckBox( i18n("Store filetype in inode") );
    m_sparse_super_check = new QCheckBox( i18n("Sparse superblock") );
    base_options_layout->addWidget(m_ext_attr_check);
    base_options_layout->addWidget(m_resize_inode_check);
    base_options_layout->addWidget(m_dir_index_check);
    base_options_layout->addWidget(m_filetype_check);
    base_options_layout->addWidget(m_sparse_super_check);

    m_extent_check      = new QCheckBox( i18n("Use extents") ); 
    m_flex_bg_check     = new QCheckBox( i18n("Flexible block group layout") );
    m_huge_file_check   = new QCheckBox( i18n("Enable files over 2TB") );
    m_uninit_bg_check   = new QCheckBox( i18n("Don't init all block groups") );
    m_lazy_itable_init_check = new QCheckBox( i18n("Don't init all inodes") );
    m_dir_nlink_check   = new QCheckBox( i18n("Unlimited subdirectories") );
    m_extra_isize_check = new QCheckBox( i18n("Nanosecond timestamps") );
    ext4_options_layout->addWidget(m_flex_bg_check);
    ext4_options_layout->addWidget(m_huge_file_check);
    ext4_options_layout->addWidget(m_uninit_bg_check);
    ext4_options_layout->addWidget(m_lazy_itable_init_check);
    ext4_options_layout->addWidget(m_dir_nlink_check);
    ext4_options_layout->addWidget(m_extra_isize_check);
    ext4_options_layout->addWidget(m_extent_check);

    m_base_options_box->setLayout(base_options_layout);
    m_ext4_options_box->setLayout(ext4_options_layout);

    left_layout->addWidget(m_base_options_box);
    right_layout->addWidget(m_ext4_options_box);
    left_layout->addStretch();
    right_layout->addStretch();

    connect(m_sparse_super_check, SIGNAL(toggled(bool)), 
            this, SLOT(setAdvancedTab(bool)));

    connect(m_inode_edit,  SIGNAL(textEdited(QString)), 
            m_total_edit, SLOT(clear()));

    connect(m_total_edit,  SIGNAL(textEdited(QString)), 
            m_inode_edit, SLOT(clear()));

    return tab;
}

void MkfsDialog::setAdvancedTab(bool)
{
    if( ext2->isChecked() || ext3->isChecked() || ext4->isChecked() ){
        m_tab_widget->setTabEnabled( 1, true );
        m_base_options_box->setEnabled(true);

        if( ext4->isChecked() )
            m_ext4_options_box->setEnabled(true);
        else{
            m_ext4_options_box->setEnabled(false);
            m_ext4_options_box->setChecked(false);
        }
    }
    else{
        m_base_options_box->setEnabled(false);
        m_base_options_box->setChecked(false);
        m_ext4_options_box->setEnabled(false);
        m_ext4_options_box->setChecked(false);
        m_tab_widget->setTabEnabled( 1, false );
    }

    if( m_sparse_super_check->isChecked() )
        m_resize_inode_check->setEnabled(true);
    else{
        m_resize_inode_check->setEnabled(false);
        m_resize_inode_check->setChecked(false);
    }
}

void MkfsDialog::clobberFS()
{
    QFile *device;
    QByteArray zero_array(128 * 1024, '\0');

    if(m_clobber_fs_check->isChecked()){
        device = new QFile(m_path);
        if( device->open(QIODevice::ReadWrite) ){ // nuke the old filesystem with zeros
            device->write(zero_array);
            device->flush();
            device->close();
        }
    }
}

QStringList MkfsDialog::arguments()
{
    QStringList arguments;
    QStringList mkfs_options;
    QStringList ext_options;
    QStringList extended_options;
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
    else if(btrfs->isChecked()){
	type = "btrfs";
    }
    else if(ntfs->isChecked()){
	type = "ntfs";
        mkfs_options << "-Q" << "--quiet";
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
    
    if( (type == "ext2") || (type == "ext3") || (type == "ext4") ){
        mkfs_options << "-m" << QString("%1").arg(m_reserved_spin->value());

        if(m_block_combo->currentIndex() > 0)
            mkfs_options << "-b" << m_block_combo->currentText().remove("KiB").trimmed();

        if( !(m_inode_edit->text()).isEmpty() )
            mkfs_options << "-i" << m_inode_edit->text();
        else if ( !(m_total_edit->text()).isEmpty() )
            mkfs_options << "-N" << m_total_edit->text();

        if( m_inode_combo->currentIndex() > 0 )
            mkfs_options << "-I" << m_inode_combo->currentText().remove("Bytes").trimmed();

        if(m_stripe_box->isChecked() || m_ext4_options_box->isChecked() ){
            mkfs_options << "-E";

            if( m_stripe_box->isChecked() ){
                extended_options << QString("stride=%1").arg( m_stride_edit->text() ); 
                extended_options << QString("stripe_width=%1").arg( m_count_edit->text() );
            }

            if( m_ext4_options_box->isChecked() ){
                if( m_lazy_itable_init_check->isChecked() )
                    extended_options << "lazy_itable_init=1";
                else
                    extended_options << "lazy_itable_init=0";
            }
            
            mkfs_options << extended_options.join(",");
        }

        if( !(m_volume_edit->text()).isEmpty() )
            mkfs_options << "-L" << m_volume_edit->text();

        if( m_base_options_box->isChecked() ){

            if( m_ext_attr_check->isChecked() )
                ext_options << "ext_attr";
            else
                ext_options << "^ext_attr";

            if( m_resize_inode_check->isChecked() )
                ext_options << "resize_inode";
            else
                ext_options << "^resize_inode"; 

            if( m_dir_index_check->isChecked() ) 
                ext_options << "dir_index"; 
            else
                ext_options << "^dir_index"; 

            if( m_filetype_check->isChecked() ) 
                ext_options << "filetype"; 
            else
                ext_options << "^filetype"; 
            
            if( m_sparse_super_check->isChecked() ) 
                ext_options << "sparse_super"; 
            else
                ext_options << "^sparse_super"; 
        }

        if( m_ext4_options_box->isChecked() && ext4->isChecked() ){

            if( m_extent_check->isChecked() )      
                ext_options << "extent";
            else
                ext_options << "^extent";

            if( m_flex_bg_check->isChecked() )      
                ext_options << "flex_bg";
            else
                ext_options << "^flex_bg";

            if( m_huge_file_check->isChecked() )      
                ext_options << "huge_file";
            else
                ext_options << "^huge_file";

            if( m_uninit_bg_check->isChecked() )      
                ext_options << "uninit_bg";
            else
                ext_options << "^uninit_bg";

            if( m_dir_nlink_check->isChecked() )      
                ext_options << "dir_nlink";
            else
                ext_options << "^dir_nlink";

            if( m_extra_isize_check->isChecked() )      
                ext_options << "extra_isize";
            else
                ext_options << "^extra_isize";
        }
    }

    if(type != "swap"){

	arguments << "mkfs" << "-t" << type;;

	if( mkfs_options.size() )
	    arguments << mkfs_options;

        if( ext_options.size() )
            arguments << "-O" << ext_options.join(",");

        arguments << m_path;
    }
    else{
	arguments << "mkswap" << m_path;
    }

    return arguments;
}

