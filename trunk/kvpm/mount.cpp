/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include "mount.h"

#include <sys/mount.h>
#include <linux/fs.h>
#include <errno.h>
#include <string.h>

#include <KTabWidget>
#include <KFileDialog>
#include <KPushButton>
#include <KMessageBox>
#include <KUrl>
#include <KLocale>

#include <QtGui>

#include "logvol.h"
#include "mountentry.h"
#include "mounttables.h"
#include "storagepartition.h"


const int BUFF_LEN = 2000;   // Enough?


MountDialog::MountDialog(LogVol *const volume, QWidget *parent) : KDialog(parent)
{
    m_device_to_mount = volume->getMapperPath();
    m_filesystem_type = volume->getFilesystem();
    m_is_writable = volume->isWritable();
    m_mount_point = volume->getFstabMountPoint();

    buildDialog();
}

MountDialog::MountDialog(StoragePartition *const partition, QWidget *parent) : KDialog(parent)
{
    m_device_to_mount = partition->getName();
    m_filesystem_type = partition->getFilesystem();
    m_is_writable = partition->isWritable();
    m_mount_point = partition->getFstabMountPoint();

    buildDialog();
}

void MountDialog::buildDialog()
{
    setCaption("Mount a Filesystem");
    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);

    QVBoxLayout *const layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    QLabel *const device_label = new QLabel( i18n("<b>Device to mount: %1</b>", m_device_to_mount) );
    device_label->setAlignment(Qt::AlignCenter);    
    layout->addWidget(device_label);
    layout->addSpacing(5);

    QHBoxLayout *const main_layout = new QHBoxLayout();
    layout->addLayout(main_layout);
    QVBoxLayout *const left_layout = new QVBoxLayout();
    main_layout->addLayout(left_layout);

    left_layout->addWidget( filesystemBox() );
    left_layout->addWidget( mountPointBox() );
    main_layout->addWidget( optionsBox() );

    connect(this, SIGNAL( accepted() ), 
	    this, SLOT( mountFilesystem() ));
}

QWidget* MountDialog::filesystemBox()
{
    QGroupBox   *const filesystem_box = new QGroupBox( i18n("Filesystem Type"), this);
    QVBoxLayout *const layout_left    = new QVBoxLayout();
    QVBoxLayout *const layout_center  = new QVBoxLayout();
    QVBoxLayout *const layout_right   = new QVBoxLayout();
    QHBoxLayout *const upper_layout   = new QHBoxLayout();
    QHBoxLayout *const lower_layout   = new QHBoxLayout();
    QVBoxLayout *const filesystem_layout = new QVBoxLayout();
    upper_layout->addLayout(layout_left);
    upper_layout->addLayout(layout_center);
    upper_layout->addLayout(layout_right);
    filesystem_layout->addLayout(upper_layout);
    filesystem_layout->addLayout(lower_layout);
    filesystem_box->setLayout(filesystem_layout);
    
    ext2_button      = new QRadioButton("ext2", this);
    ext3_button      = new QRadioButton("ext3", this);
    ext4_button      = new QRadioButton("ext4", this);
    btrfs_button     = new QRadioButton("btrfs",this);
    reiserfs3_button = new QRadioButton("reiser3", this);
    reiserfs4_button = new QRadioButton("reiser4", this);
    xfs_button       = new QRadioButton("xfs", this);
    jfs_button       = new QRadioButton("jfs", this);
    vfat_button      = new QRadioButton("vfat",this);
    udf_button       = new QRadioButton("udf", this);
    iso9660_button   = new QRadioButton("iso9660", this);
    hfs_button       = new QRadioButton("hfs", this);
    ntfs_button      = new QRadioButton("ntfs", this);
    
    specify_button   = new QRadioButton( i18n("Specify another fileystem:"), this);

    if( m_filesystem_type == "ext2" )
	ext2_button->setChecked(true);
    else if( m_filesystem_type == "ext3" )
	ext3_button->setChecked(true);
    else if( m_filesystem_type == "ext4" )
	ext4_button->setChecked(true);
    else if( m_filesystem_type == "btrfs" )
	btrfs_button->setChecked(true);
    else if( m_filesystem_type == "reiserfs" )
	reiserfs3_button->setChecked(true);
    else if( m_filesystem_type == "reiser4" )
	reiserfs4_button->setChecked(true);
    else if( m_filesystem_type == "xfs" )
	xfs_button->setChecked(true);
    else if( m_filesystem_type == "jfs" )
	jfs_button->setChecked(true);
    else if( m_filesystem_type == "vfat" )
	vfat_button->setChecked(true);
    else if( m_filesystem_type == "udf" )
	udf_button->setChecked(true);
    else if( m_filesystem_type == "iso9660" )
	iso9660_button->setChecked(true);
    else if( m_filesystem_type == "hfs" )
	hfs_button->setChecked(true);
    else if( m_filesystem_type == "ntfs" )
	ntfs_button->setChecked(true);
    else
	specify_button->setChecked(true);

    m_filesystem_edit = new KLineEdit(m_filesystem_type);

    if( specify_button->isChecked() && (m_filesystem_edit->text()).isEmpty() )
	(button(KDialog::Ok))->setEnabled(false);
    
    layout_left->addWidget(ext2_button);
    layout_left->addWidget(ext3_button);
    layout_left->addWidget(ext4_button);
    layout_left->addWidget(btrfs_button);
    layout_left->addWidget(ntfs_button);
    layout_left->addStretch();
    layout_center->addWidget(reiserfs3_button);
    layout_center->addWidget(reiserfs4_button);
    layout_center->addWidget(xfs_button);
    layout_center->addWidget(jfs_button);
    layout_center->addStretch();
    layout_right->addWidget(udf_button);
    layout_right->addWidget(iso9660_button);
    layout_right->addWidget(hfs_button);
    layout_right->addWidget(vfat_button);
    layout_right->addStretch();    

    lower_layout->addWidget(specify_button);
    lower_layout->addWidget(m_filesystem_edit);

    connect(m_filesystem_edit, SIGNAL( textEdited(const QString) ), 
	    this, SLOT( toggleOKButton(const QString) ));

    connect(specify_button, SIGNAL( toggled(bool) ), 
	    this, SLOT( toggleOKButton(bool) ));

    return filesystem_box;
}

QWidget* MountDialog::optionsBox()
{
    QWidget *options_box = new QGroupBox( i18n("Mount Options") );
    QVBoxLayout *options_layout = new QVBoxLayout();
    options_box->setLayout(options_layout);
    
    QGroupBox *common_options_box = new QGroupBox( i18n("Common Options"), this);
    QVBoxLayout *layout_left      = new QVBoxLayout();
    QVBoxLayout *layout_right     = new QVBoxLayout();
    QHBoxLayout *common_options_layout = new QHBoxLayout();
    common_options_layout->addLayout(layout_left);
    common_options_layout->addLayout(layout_right);
    common_options_box->setLayout(common_options_layout);

    sync_check     = new QCheckBox("sync");
    dirsync_check  = new QCheckBox("dirsync");
    rw_check       = new QCheckBox("rw");
    suid_check     = new QCheckBox("suid");
    dev_check      = new QCheckBox("dev");
    exec_check     = new QCheckBox("exec");
    mand_check     = new QCheckBox("mand");
    acl_check      = new QCheckBox("acl");
    user_xattr_check = new QCheckBox("user xattr");
    
    if(m_is_writable)
        rw_check->setChecked(true);
    else{
        rw_check->setChecked(false);
        rw_check->setEnabled(false);
    }

    suid_check->setChecked(true);
    dev_check->setChecked(true);
    exec_check->setChecked(true);
    
    sync_check->setToolTip( i18n("Always use synchronous I/O") );
    rw_check->setToolTip( i18n("Allow writing in addition to reading") );
    suid_check->setToolTip( i18n("Allow the suid bit to have effect") );
    dev_check->setToolTip( i18n("Allow the use of block and special devices") );
    exec_check->setToolTip( i18n("Allow the execution of binary files") );
    mand_check->setToolTip( i18n("Allow manditory file locks") );
    acl_check->setToolTip( i18n("Allow use of access control lists") );
    
    layout_left->addWidget(rw_check);
    layout_left->addWidget(suid_check);
    layout_left->addWidget(sync_check);
    layout_left->addWidget(dirsync_check);
    layout_left->addWidget(acl_check);
    layout_left->addStretch();
    layout_right->addWidget(dev_check);
    layout_right->addWidget(exec_check);
    layout_right->addWidget(mand_check);
    layout_right->addWidget(user_xattr_check);
    layout_right->addStretch();
    
    QHBoxLayout *options_atime_layout = new QHBoxLayout();
    options_layout->addLayout(options_atime_layout);

    QGroupBox *atime_box = new QGroupBox( i18n("Update atime"), this);
    QVBoxLayout *atime_layout = new QVBoxLayout();
    atime_box->setLayout(atime_layout);
    options_atime_layout->addWidget(common_options_box);

    QVBoxLayout *atime_journal_layout = new QVBoxLayout();
    options_atime_layout->addLayout(atime_journal_layout);

    m_filesystem_journal_box = new QGroupBox( i18n("Journaling") );
    QVBoxLayout *journaling_layout = new QVBoxLayout;
    m_filesystem_journal_box->setLayout(journaling_layout);
    data_journal_button   = new QRadioButton("data=journal");
    data_ordered_button   = new QRadioButton("data=ordered");
    data_ordered_button->setChecked(true);
    data_writeback_button = new QRadioButton("data=writeback");
    journaling_layout->addWidget(data_journal_button);
    journaling_layout->addWidget(data_ordered_button);
    journaling_layout->addWidget(data_writeback_button);
    atime_journal_layout->addWidget(m_filesystem_journal_box);

    atime_button      = new QRadioButton("atime");
    noatime_button    = new QRadioButton("noatime");
    nodiratime_button = new QRadioButton("nodiratime");
    relatime_button   = new QRadioButton("relatime");
    relatime_button->setChecked(true);
    atime_layout->addWidget(atime_button);
    atime_layout->addWidget(noatime_button);
    atime_layout->addWidget(nodiratime_button);
    atime_layout->addWidget(relatime_button);

    atime_button->setToolTip( i18n("Always update atime, this is the default") );
    noatime_button->setToolTip( i18n("Do not update atime") );
    nodiratime_button->setToolTip( i18n("Do not update atime for directory access") );
    relatime_button->setToolTip( i18n("Access time is only updated if the previous "
				      "access time was earlier than the current modify "
				      "or change time") );
    
    atime_journal_layout->addWidget(atime_box);

    QGroupBox *filesystem_options_box = new QGroupBox( i18n("Filesystem specific mount options") );
    m_fs_specific_edit = new KLineEdit();
    m_fs_specific_edit->setPlaceholderText( i18n("comma separated list of additional mount options") );

    options_layout->addWidget(filesystem_options_box);
    QVBoxLayout *filesystem_options_box_layout = new QVBoxLayout();
    
    filesystem_options_box->setLayout(filesystem_options_box_layout);
    filesystem_options_box_layout->addWidget(m_fs_specific_edit);

    toggleAdditionalOptions(true);

    connect(ext2_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));
    
    connect(ext3_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));
    
    connect(ext4_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));
    
    connect(reiserfs3_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));

    connect(reiserfs4_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));
    
    connect(btrfs_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));
    
    connect(xfs_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));

    connect(jfs_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));
    
    connect(vfat_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));

    connect(ntfs_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));
    
    connect(specify_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));
    
    connect(udf_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));
    
    connect(iso9660_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));
    
    connect(hfs_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));
    
    return options_box;
}

QWidget* MountDialog::mountPointBox()
{
    QGroupBox *const mount_point_box = new QGroupBox( i18n("Mount Point") );
    QHBoxLayout *const mount_point_layout = new QHBoxLayout();
    mount_point_box->setLayout(mount_point_layout);
    
    m_mount_point_edit = new KLineEdit(m_mount_point);

    if( (m_mount_point_edit->text()).isEmpty() )
    	(button(KDialog::Ok))->setEnabled(false);

    mount_point_layout->addWidget(m_mount_point_edit);
    
    QPushButton *const browse_button = new QPushButton( i18n("Browse"), this);
    mount_point_layout->addWidget(browse_button);

    connect(m_mount_point_edit, SIGNAL( textChanged(const QString) ), 
	    this, SLOT( toggleOKButton(const QString) ));

    connect(browse_button, SIGNAL( clicked(bool) ), 
	    this, SLOT( selectMountPoint(bool) ));

    return mount_point_box;
}

void MountDialog::selectMountPoint(bool)
{
    const QString filter = "*";
    QString start;

    if( !m_mount_point.isEmpty() )
	start = QString("file://" + m_mount_point);
    else
	start = QString("file:///");

    const KUrl url(start);
    KFileDialog dialog( url, filter, 0 );
    dialog.setModal(true);
    dialog.setMode(KFile::Directory);
    dialog.exec();
    m_mount_point = dialog.selectedFile();
    
    if( m_mount_point.length() > 1 && m_mount_point.endsWith('/') )
	m_mount_point.chop(1);                           // remove trailing "/" character

    m_mount_point_edit->setText(m_mount_point);
}

void MountDialog::mountFilesystem()
{
    unsigned long options = 0;
    QStringList standard_options, additional_options;
    QString all_options; 
    
    if( ext2_button->isChecked() )
	m_filesystem_type = "ext2";
    else if( ext3_button->isChecked() )
	m_filesystem_type = "ext3";
    else if( ext4_button->isChecked() )
	m_filesystem_type = "ext4";
    else if( btrfs_button->isChecked() )
	m_filesystem_type = "btrfs";
    else if( reiserfs3_button->isChecked() )
	m_filesystem_type = "reiserfs";
    else if( reiserfs4_button->isChecked() )
	m_filesystem_type = "reiser4";
    else if( xfs_button->isChecked() )
	m_filesystem_type = "xfs";
    else if( jfs_button->isChecked() )
	m_filesystem_type = "jfs";
    else if( vfat_button->isChecked() )
	m_filesystem_type = "vfat";
    else if( ntfs_button->isChecked() )
	m_filesystem_type = "ntfs";
    else
	m_filesystem_type = m_filesystem_edit->text();

    m_mount_point = m_mount_point_edit->text();

    if( m_mount_point.length() > 1 && m_mount_point.endsWith('/') )  // remove trailing slash
	m_mount_point.chop(1);

    if( sync_check->isChecked() ){
	standard_options.append("sync");
	options |= MS_SYNCHRONOUS;
    }
    if( dirsync_check->isChecked() ){
	standard_options.append("dirsync");
	options |= MS_DIRSYNC;
    }
    if( rw_check->isChecked() ){
	standard_options.append("rw");
    }
    else{
	standard_options.append("ro");
	options |= MS_RDONLY;
    }

    if( !suid_check->isChecked() ){
	standard_options.append("nosuid");
	options |= MS_NOSUID;
    }
    if( !dev_check->isChecked() ){
	standard_options.append("nodev");
	options  |= MS_NODEV;
    }
    if( !exec_check->isChecked() ){
	standard_options.append("noexec");
	options |= MS_NOEXEC;
    }
    if( mand_check->isChecked() ){
	standard_options.append("mand");
	options |= MS_MANDLOCK;
    }

    if( atime_button->isChecked() ){
	standard_options.append("atime");
	options |= MS_STRICTATIME;
    }
    else if( noatime_button->isChecked() ){
	standard_options.append("noatime");
	options |= MS_NOATIME;
    }
    else if( nodiratime_button->isChecked() ){
	standard_options.append("nodiratime");
	options |= MS_NODIRATIME;
    }
    else if( relatime_button->isChecked() ){
	standard_options.append("relatime");
	options |= MS_RELATIME;
    }

/* "data=ordered" is the default so we ignore that button */

    if( m_filesystem_journal_box->isEnabled() ){
	if( data_journal_button->isChecked() ) 
	    additional_options.append("data=journal");
	else if( data_writeback_button->isChecked() ) 
	    additional_options.append("data=writeback");
    }

    if( acl_check->isEnabled() ){
        if( acl_check->isChecked() )
            additional_options.append("acl");
        else
            additional_options.append("noacl");
    }

    if( user_xattr_check->isEnabled() ){
        if( user_xattr_check->isChecked() )
            additional_options.append("user_xattr");
        else
            additional_options.append("nouser_xattr");
    }

    if( !m_fs_specific_edit->text().trimmed().isEmpty() )
	additional_options.append( (m_fs_specific_edit->text()).trimmed() );
    
    all_options = standard_options.join(",");

    if( additional_options.size() ){
	all_options.append(",");
	all_options.append( additional_options.join(",") );
    }

    const QByteArray device      = m_device_to_mount.toLocal8Bit();
    const QByteArray mount_point = m_mount_point.toLocal8Bit();
    const QByteArray fs_type     = m_filesystem_type.toLocal8Bit();
    const QByteArray fs_options  = additional_options.join(",").toLocal8Bit();

    int error = mount( device.data(), mount_point.data(), fs_type.data(), options, fs_options.data() );

    if( !error )
        MountTables::addEntry(m_device_to_mount, m_mount_point, m_filesystem_type, all_options, 0, 0);
    else
	KMessageBox::error(0, QString("Error number: %1  %2").arg(errno).arg(strerror(errno)));
}

void MountDialog::toggleOKButton()
{
    if ( (specify_button->isChecked() && (m_filesystem_edit->text()).isEmpty()) || (m_mount_point_edit->text()).isEmpty())
    	(button(KDialog::Ok))->setEnabled(false);
    else
    	(button(KDialog::Ok))->setEnabled(true);
}

void MountDialog::toggleOKButton(const QString)
{
    toggleOKButton();
}

void MountDialog::toggleOKButton(bool)
{
    toggleOKButton();
}

void MountDialog::toggleAdditionalOptions(bool)
{
    if( ext3_button->isChecked() )
        m_filesystem_journal_box->setEnabled(true);
    else
        m_filesystem_journal_box->setEnabled(false);

    if( ext2_button->isChecked() || ext3_button->isChecked() || ext4_button->isChecked() || 
	reiserfs3_button->isChecked() || reiserfs4_button->isChecked() ){

	acl_check->setEnabled(true);
	acl_check->setChecked(true);
	user_xattr_check->setEnabled(true);
	user_xattr_check->setChecked(true);
    }
    else{
	acl_check->setEnabled(false);
	acl_check->setChecked(false);
	user_xattr_check->setEnabled(false);
	user_xattr_check->setChecked(false);
    }
}

