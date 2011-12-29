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
#include <KLocale>

#include <QtGui>

#include "logvol.h"
#include "misc.h"
#include "processprogress.h"
#include "storagepartition.h"


MkfsDialog::MkfsDialog(LogVol *const volume, QWidget *parent) : KDialog(parent)
{
    m_path = volume->getMapperPath();

    const long stride_size = 1; // volume->getSegmentStripeSize( 0 ); <-- must convert to blocks!
    const long stride_count = volume->getSegmentStripes(0);

    if( hasInitialErrors( volume->isMounted() ) )
        m_bailout = true;
    else{
        m_bailout = false;
        buildDialog(stride_size, stride_count);
    }
}

MkfsDialog::MkfsDialog(StoragePartition *const partition, QWidget *parent) : KDialog(parent)
{
    m_path = partition->getName();

    const long stride_size = 1;
    const long stride_count = 1;

    if( hasInitialErrors( partition->isMounted() ) )
        m_bailout = true;
    else{
        m_bailout = false;
        buildDialog(stride_size, stride_count);
    }
}

// Determines if there is any point to calling up the dialog at all
bool MkfsDialog::hasInitialErrors(const bool mounted)
{
    const QString warning_message = i18n("Writing a new file system on <b>%1</b> "
                                         "will delete any existing data on it.", m_path);
    
    const QString error_message = i18n("The volume: <b>%1</b> is mounted. It must be "
                                       "unmounted before a new filesystem " 
                                       "can be written on it", m_path);

    if(mounted){
        KMessageBox::error(0, error_message);
        return true;
    }
    
    if(KMessageBox::warningContinueCancel(0, warning_message) == KMessageBox::Continue)
        return false;

    return true;
}

void MkfsDialog::buildDialog(const long strideSize, const long strideCount)
{
    QWidget *const dialog_body = new QWidget;
    QVBoxLayout *const layout = new QVBoxLayout;
    dialog_body->setLayout(layout);

    QLabel *const label = new QLabel( i18n("<b>Write filesystem on: %1</b>", m_path) );
    label->setAlignment(Qt::AlignCenter);
    layout->addSpacing(5);
    layout->addWidget(label);
    layout->addSpacing(5);

    m_tab_widget = new KTabWidget(this);
    m_tab_widget->addTab(generalTab(), i18n("Filesystem Type") );
    m_tab_widget->addTab(advancedTab(strideSize, strideCount), i18n("Standard Ext Options") );
    m_tab_widget->addTab(ext4Tab(), i18n("Additional Ext4 Options") );
    layout->addWidget(m_tab_widget);

    setMainWidget(dialog_body);
    setCaption( i18n("Write Filesystem") );

    enableOptions(true);

    connect(this, SIGNAL(okClicked()), 
            this, SLOT(commitFilesystem()));
}

QWidget* MkfsDialog::generalTab()
{
    QWidget *const tab = new QWidget();
    QVBoxLayout *const layout = new QVBoxLayout();
    QHBoxLayout *const upper_layout = new QHBoxLayout();
    QHBoxLayout *const lower_layout = new QHBoxLayout();
    layout->addLayout(upper_layout);
    layout->addLayout(lower_layout);

    QGroupBox *const radio_box = new QGroupBox( i18n("Select Filesystem") );
    QGridLayout *const radio_layout = new QGridLayout;
    radio_box->setLayout(radio_layout);
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

    upper_layout->addWidget(radio_box);
    lower_layout->addStretch();

    QHBoxLayout *const name_layout = new QHBoxLayout;
    QLabel *const name_label = new QLabel( i18n("Optional name or label: ") );
    name_layout->addWidget(name_label);
    m_name_edit = new KLineEdit();
    name_label->setBuddy(m_name_edit);
    name_layout->addWidget(m_name_edit);
    name_layout->addStretch();
    lower_layout->addLayout(name_layout);

    lower_layout->addStretch();
    layout->addStretch();
    tab->setLayout(layout);

    connect(ext2, SIGNAL(toggled(bool)), 
            this, SLOT(enableOptions(bool)));

    connect(ext3, SIGNAL(toggled(bool)), 
            this, SLOT(enableOptions(bool)));

    connect(ext4, SIGNAL(toggled(bool)), 
            this, SLOT(enableOptions(bool)));

    return tab;
}

QWidget* MkfsDialog::advancedTab(const long strideSize, const long strideCount)
{
    QWidget *const tab = new QWidget();

    QVBoxLayout *const layout = new QVBoxLayout();
    QHBoxLayout *const top_layout = new QHBoxLayout();
    top_layout->addStretch();
    top_layout->addLayout(layout);
    top_layout->addStretch();

    QLabel *const override_label = new QLabel( i18n("If enabled, these options override the mkfs defaults "
                                                    "for ext2, ext3 and ext4 filesystems.") );

    QHBoxLayout *const label_layout = new QHBoxLayout();
    layout->addSpacing(10);
    layout->addLayout(label_layout);
    layout->addSpacing(10);

    override_label->setWordWrap(true);
    override_label->setAlignment(Qt::AlignCenter);
    label_layout->addSpacing(40);
    label_layout->addWidget(override_label);
    label_layout->addSpacing(40);

    QHBoxLayout *const lower_layout = new QHBoxLayout();
    layout->addLayout(lower_layout);

    QVBoxLayout *const left_layout = new QVBoxLayout;
    QVBoxLayout *const right_layout = new QVBoxLayout;
    lower_layout->addLayout(left_layout);
    lower_layout->addLayout(right_layout);

    m_stripe_box = stripeBox(strideSize, strideCount);
    right_layout->addWidget(m_stripe_box);

    m_misc_options_box = miscOptionsBox();
    m_base_options_box = baseOptionsBox();
    left_layout->addWidget(m_base_options_box);
    right_layout->addWidget(m_misc_options_box);

    left_layout->addStretch();
    right_layout->addStretch();

    connect(m_sparse_super_check, SIGNAL(toggled(bool)), 
            this, SLOT(enableOptions(bool)));

    connect(m_inode_edit, SIGNAL(textEdited(QString)), 
            m_total_edit, SLOT(clear()));

    connect(m_total_edit, SIGNAL(textEdited(QString)), 
            m_inode_edit, SLOT(clear()));

    tab->setLayout(top_layout);

    return tab;
}

QWidget* MkfsDialog::ext4Tab()
{
    QWidget *const tab = new QWidget();
    QVBoxLayout *const layout = new QVBoxLayout();
    QHBoxLayout *const lower_layout = new QHBoxLayout();

    QLabel *const override_label = new QLabel( i18n("If enabled, these options override the mkfs defaults "
                                                    "for settings that only apply to ext4 filesystems.") );

    QHBoxLayout *const label_layout = new QHBoxLayout();
    layout->addSpacing(10);
    layout->addLayout(label_layout);
    layout->addSpacing(10);

    override_label->setWordWrap(true);
    override_label->setAlignment(Qt::AlignCenter);

    label_layout->addSpacing(40);
    label_layout->addWidget(override_label);
    label_layout->addSpacing(40);

    m_ext4_options_box = ext4OptionsBox();

    lower_layout->addStretch();
    lower_layout->addWidget(m_ext4_options_box);
    lower_layout->addStretch();

    layout->addStretch();
    layout->addLayout(lower_layout);
    layout->addStretch();

    tab->setLayout(layout);
    return tab;
}

QGroupBox* MkfsDialog::miscOptionsBox()
{
    QGroupBox *const misc_box = new QGroupBox("Inodes and Blocks");
    misc_box->setCheckable(true);
    misc_box->setChecked(false);

    QVBoxLayout *const misc_layout = new QVBoxLayout;

    QHBoxLayout *const reserved_layout = new QHBoxLayout;
    QHBoxLayout *const block_layout  = new QHBoxLayout;
    QHBoxLayout *const isize_layout  = new QHBoxLayout;
    QHBoxLayout *const inode_layout  = new QHBoxLayout;
    QHBoxLayout *const total_layout  = new QHBoxLayout;

    QLabel *label;

    label = new QLabel( i18n("Reserved space: ") );
    reserved_layout->addWidget(label);
    m_reserved_spin = new QSpinBox;
    label->setBuddy(m_reserved_spin);
    m_reserved_spin->setRange(0, 100);
    m_reserved_spin->setValue(5);
    m_reserved_spin->setPrefix("%");
    reserved_layout->addWidget(m_reserved_spin);
    reserved_layout->addStretch();
    misc_layout->addLayout(reserved_layout);

    label = new QLabel( i18n("Block size: ") );
    block_layout->addWidget(label);
    m_block_combo = new KComboBox();
    label->setBuddy(m_block_combo);
    m_block_combo->insertItem(0, i18nc("Let the program decide", "default") );
    m_block_combo->insertItem(1,"1024 KiB");
    m_block_combo->insertItem(2,"2048 KiB");
    m_block_combo->insertItem(3,"4096 KiB");
    m_block_combo->setInsertPolicy(KComboBox::NoInsert);
    m_block_combo->setCurrentIndex(0);
    block_layout->addWidget(m_block_combo);
    block_layout->addStretch();
    misc_layout->addLayout(block_layout);

    label = new QLabel( i18n("Inode size: ") );
    isize_layout->addWidget(label);
    m_inode_combo = new KComboBox();
    label->setBuddy(m_inode_combo);
    m_inode_combo->insertItem(0, i18nc("Let the program decide", "default") );
    m_inode_combo->insertItem(1,"128 Bytes");
    m_inode_combo->insertItem(2,"256 Bytes");
    m_inode_combo->insertItem(3,"512 Bytes");
    m_inode_combo->setInsertPolicy(KComboBox::NoInsert);
    m_inode_combo->setCurrentIndex(0);
    isize_layout->addWidget(m_inode_combo);
    isize_layout->addStretch();
    misc_layout->addLayout(isize_layout);

    label = new QLabel( i18n("Bytes / inode: ") );
    inode_layout->addWidget(label);
    m_inode_edit = new KLineEdit();
    m_inode_edit->setPlaceholderText(i18nc("Let the program decide", "default") );
    label->setBuddy(m_inode_edit);
    QIntValidator *inode_validator = new QIntValidator(m_inode_edit);
    m_inode_edit->setValidator(inode_validator);
    inode_validator->setBottom(0);
    inode_layout->addWidget(m_inode_edit);
    misc_layout->addLayout(inode_layout);

    label = new QLabel( i18n("Total inodes: ") );
    total_layout->addWidget(label);
    m_total_edit = new KLineEdit();
    m_total_edit->setPlaceholderText(i18nc("Let the program decide", "default") );
    label->setBuddy(m_total_edit);
    QIntValidator *const total_validator = new QIntValidator(m_total_edit);
    m_total_edit->setValidator(total_validator);
    total_validator->setBottom(0);
    total_layout->addWidget(m_total_edit);
    misc_layout->addLayout(total_layout);

    misc_box->setLayout(misc_layout);

    return misc_box;
}

QGroupBox* MkfsDialog::ext4OptionsBox()
{
    QGroupBox *const options_box = new QGroupBox("Ext4 Options");
    QVBoxLayout *const options_layout = new QVBoxLayout();
    options_box->setCheckable(true);
    options_box->setChecked(false);

    m_extent_check      = new QCheckBox( i18n("Use extents") ); 
    m_flex_bg_check     = new QCheckBox( i18n("Flexible block group layout") );
    m_huge_file_check   = new QCheckBox( i18n("Enable files over 2TB") );
    m_uninit_bg_check   = new QCheckBox( i18n("Don't init all block groups") );
    m_lazy_itable_init_check = new QCheckBox( i18n("Don't init all inodes") );
    m_dir_nlink_check   = new QCheckBox( i18n("Unlimited subdirectories") );
    m_extra_isize_check = new QCheckBox( i18n("Nanosecond timestamps") );
    options_layout->addWidget(m_flex_bg_check);
    options_layout->addWidget(m_huge_file_check);
    options_layout->addWidget(m_uninit_bg_check);
    options_layout->addWidget(m_lazy_itable_init_check);
    options_layout->addWidget(m_dir_nlink_check);
    options_layout->addWidget(m_extra_isize_check);
    options_layout->addWidget(m_extent_check);

    options_box->setLayout(options_layout);

    return options_box;
}

QGroupBox* MkfsDialog::stripeBox(const long strideSize, const long strideCount)
{
    QGroupBox *const stripe_box = new QGroupBox( i18n("Striping") );
    stripe_box->setCheckable(true);
    stripe_box->setChecked(false);
    QVBoxLayout *const stripe_layout = new QVBoxLayout();
    stripe_box->setLayout(stripe_layout);

    QLabel *label;

    QHBoxLayout *const stride_layout = new QHBoxLayout;
    label = new QLabel( i18n("Stride size in blocks: ") );
    stride_layout->addWidget(label);
    m_stride_edit = new KLineEdit( QString("%1").arg(strideSize) );
    label->setBuddy(m_stride_edit);
    QIntValidator *const stride_validator = new QIntValidator(m_stride_edit);
    m_stride_edit->setValidator(stride_validator);
    stride_validator->setBottom(512);
    stride_layout->addWidget(m_stride_edit);
    stride_layout->addStretch();
    stripe_layout->addLayout(stride_layout);

    QHBoxLayout *const count_layout  = new QHBoxLayout;
    label = new QLabel( i18n("Strides per stripe: ") );
    count_layout->addWidget(label);
    m_count_edit = new KLineEdit( QString("%1").arg(strideCount) );
    label->setBuddy(m_count_edit);
    QIntValidator *const count_validator = new QIntValidator(m_count_edit);
    m_count_edit->setValidator(count_validator);
    count_validator->setBottom(1);
    count_layout->addWidget(m_count_edit);
    count_layout->addStretch();
    stripe_layout->addLayout(count_layout);

    return stripe_box;
}

QGroupBox* MkfsDialog::baseOptionsBox()
{
    QGroupBox *const options_box = new QGroupBox("Basic Options");
    QVBoxLayout *const options_layout = new QVBoxLayout();
    options_box->setCheckable(true);
    options_box->setChecked(false);

    m_ext_attr_check     = new QCheckBox( i18n("Extended attributes") );
    m_resize_inode_check = new QCheckBox( i18n("Resize inode") );
    m_resize_inode_check->setEnabled(false);
    m_dir_index_check    = new QCheckBox( i18n("Directory B-Tree index") );
    m_filetype_check     = new QCheckBox( i18n("Store filetype in inode") );
    m_sparse_super_check = new QCheckBox( i18n("Sparse superblock") );
    options_layout->addWidget(m_ext_attr_check);
    options_layout->addWidget(m_resize_inode_check);
    options_layout->addWidget(m_dir_index_check);
    options_layout->addWidget(m_filetype_check);
    options_layout->addWidget(m_sparse_super_check);

    options_box->setLayout(options_layout);

    return options_box;
}

void MkfsDialog::enableOptions(bool)
{
    if( ext2->isChecked() || ext3->isChecked() || ext4->isChecked() ){
        m_tab_widget->setTabEnabled(1, true);
        m_base_options_box->setEnabled(true);
        m_misc_options_box->setEnabled(true);

        if( ext4->isChecked() ){
            m_ext4_options_box->setEnabled(true);
            m_tab_widget->setTabEnabled(2, true);
        }
        else{
            m_ext4_options_box->setEnabled(false);
            m_ext4_options_box->setChecked(false);
            m_tab_widget->setTabEnabled(2, false);
        }
    }
    else{
        m_base_options_box->setEnabled(false);
        m_base_options_box->setChecked(false);
        m_ext4_options_box->setEnabled(false);
        m_ext4_options_box->setChecked(false);
        m_misc_options_box->setEnabled(false);
        m_tab_widget->setTabEnabled(1, false);
        m_tab_widget->setTabEnabled(2, false);
    }

    if( m_sparse_super_check->isChecked() )
        m_resize_inode_check->setEnabled(true);
    else{
        m_resize_inode_check->setEnabled(false);
        m_resize_inode_check->setChecked(false);
    }
}

void MkfsDialog::clobberFilesystem()
{
    if( !isBusy(m_path) ){  // Last check -- just to be sure

        QByteArray zero_array(128 * 1024, '\0');
        
        if(m_clobber_fs_check->isChecked()){
            QFile *const device = new QFile(m_path);
            if( device->open(QIODevice::ReadWrite) ){ // nuke the old filesystem with zeros
                device->write(zero_array);
                device->flush();
                device->close();
            }
        }
    }
}

void MkfsDialog::commitFilesystem()
{
    hide();
    clobberFilesystem();

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
    
    if(type == "reiserfs"){
        if( !(m_name_edit->text()).isEmpty() )
            mkfs_options << "--label" << m_name_edit->text();
    }
    else if(type == "vfat"){
        if( !(m_name_edit->text()).isEmpty() )
            mkfs_options << "-n" << m_name_edit->text();
    }
    else if( (type == "reiser4") || (type == "jfs") || (type == "xfs") || (type == "btrfs") || (type == "ntfs") ){
        if( !(m_name_edit->text()).isEmpty() )
            mkfs_options << "-L" << m_name_edit->text();
    }
    else if( (type == "ext2") || (type == "ext3") || (type == "ext4") ){

        if( m_misc_options_box->isChecked() ){
            mkfs_options << "-m" << QString("%1").arg(m_reserved_spin->value());

            if(m_block_combo->currentIndex() > 0)
                mkfs_options << "-b" << m_block_combo->currentText().remove("KiB").trimmed();

            if( !m_inode_edit->text().isEmpty() )
                mkfs_options << "-i" << m_inode_edit->text();
            else if ( !m_total_edit->text().isEmpty() )
                mkfs_options << "-N" << m_total_edit->text();
            
            if( m_inode_combo->currentIndex() > 0 )
                mkfs_options << "-I" << m_inode_combo->currentText().remove("Bytes").trimmed();
        }

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

        if( !(m_name_edit->text()).isEmpty() )
            mkfs_options << "-L" << m_name_edit->text();

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

    ProcessProgress mkfs(arguments, true);
}

bool MkfsDialog::bailout()
{
    return m_bailout;
}
