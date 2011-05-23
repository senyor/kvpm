/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


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
#include "mount.h"
#include "mountentry.h"
#include "storagepartition.h"


const int BUFF_LEN = 2000;   // Enough?


MountDialog::MountDialog(QString deviceToMount, QString filesystemType, bool writable, QWidget *parent) : 
    KDialog(parent),
    m_device_to_mount(deviceToMount),
    m_filesystem_type(filesystemType),
    m_is_writable(writable)
{

    KTabWidget *dialog_body = new KTabWidget(this);
    setMainWidget(dialog_body);

    QWidget *main_tab    = new QWidget(this);
    QVBoxLayout *main_layout = new QVBoxLayout();
    main_tab->setLayout(main_layout);
    QLabel *device_label = new QLabel( "<b>" + deviceToMount + "</b>");
    device_label->setAlignment(Qt::AlignCenter);    
    main_layout->addWidget( device_label );
    main_layout->addWidget( filesystemBox() );
    main_layout->addWidget( mountPointBox() );

    dialog_body->addTab(main_tab,     i18n("Main") );
    dialog_body->addTab(optionsTab(), i18n("Options") );

    connect(this, SIGNAL( accepted() ), 
	    this, SLOT( mountFilesystem() ));
}

QWidget* MountDialog::filesystemBox()
{
    QGroupBox   *filesystem_box = new QGroupBox( i18n("Filesystem type"), this);
    QVBoxLayout *layout_left    = new QVBoxLayout();
    QVBoxLayout *layout_center  = new QVBoxLayout();
    QVBoxLayout *layout_right   = new QVBoxLayout();
    QHBoxLayout *upper_layout   = new QHBoxLayout();
    QHBoxLayout *lower_layout   = new QHBoxLayout();
    QVBoxLayout *filesystem_layout   = new QVBoxLayout();
    upper_layout->addLayout(layout_left);
    upper_layout->addLayout(layout_center);
    upper_layout->addLayout(layout_right);
    filesystem_layout->addLayout(upper_layout);
    filesystem_layout->addLayout(lower_layout);
    filesystem_box->setLayout(filesystem_layout);
    
    ext2_button      = new QRadioButton("Ext2", this);
    ext3_button      = new QRadioButton("Ext3", this);
    ext4_button      = new QRadioButton("Ext4", this);
    btrfs_button     = new QRadioButton("Btrfs",this);
    reiserfs3_button = new QRadioButton("Reiserfs Ver. 3", this);
    reiserfs4_button = new QRadioButton("Reiserfs Ver. 4", this);
    xfs_button       = new QRadioButton("Xfs", this);
    jfs_button       = new QRadioButton("Jfs", this);
    vfat_button      = new QRadioButton("vfat",this);
    udf_button       = new QRadioButton("udf", this);
    iso9660_button   = new QRadioButton("iso9660", this);
    hfs_button       = new QRadioButton("hfs", this);
    
    specify_button   = new QRadioButton( i18n("Specify other:"), this);

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
    else
	specify_button->setChecked(true);

    m_filesystem_edit = new KLineEdit(m_filesystem_type);

    if( specify_button->isChecked() && m_filesystem_edit->text() == "" )
	(button(KDialog::Ok))->setEnabled(false);
    
    layout_left->addWidget(ext2_button);
    layout_left->addWidget(ext3_button);
    layout_left->addWidget(ext4_button);
    layout_left->addWidget(btrfs_button);
    layout_center->addWidget(reiserfs3_button);
    layout_center->addWidget(reiserfs4_button);
    layout_center->addWidget(xfs_button);
    layout_center->addWidget(jfs_button);
    layout_right->addWidget(udf_button);
    layout_right->addWidget(iso9660_button);
    layout_right->addWidget(hfs_button);
    layout_right->addWidget(vfat_button);
    
    lower_layout->addWidget(specify_button);
    lower_layout->addWidget(m_filesystem_edit);

    connect(m_filesystem_edit, SIGNAL( textEdited(const QString) ), 
	    this, SLOT( toggleOKButton(const QString) ));

    connect(specify_button, SIGNAL( toggled(bool) ), 
	    this, SLOT( toggleOKButton(bool) ));

    return filesystem_box;
}

QWidget* MountDialog::optionsTab()
{

    QWidget *options_tab = new QWidget();
    QVBoxLayout *options_layout = new QVBoxLayout();
    options_tab->setLayout(options_layout);
    
    QGroupBox *common_options_box = new QGroupBox( i18n("Common mount options"), this);
    QVBoxLayout *layout_left      = new QVBoxLayout();
    QVBoxLayout *layout_right     = new QVBoxLayout();
    QHBoxLayout *common_options_layout   = new QHBoxLayout();
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
    options_atime_layout->addWidget(m_filesystem_journal_box);

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
    
    options_atime_layout->addWidget(atime_box);

    QGroupBox *filesystem_options_box = new QGroupBox( i18n("Filesystem specific  mount options") );
    m_fs_specific_edit = new KLineEdit();
    QLabel *additional_options_label = new QLabel( i18n("comma separated list of additional mount options") );
    
    options_layout->addWidget(filesystem_options_box);
    QVBoxLayout *filesystem_options_box_layout = new QVBoxLayout();
    
    filesystem_options_box->setLayout(filesystem_options_box_layout);
    filesystem_options_box_layout->addWidget(additional_options_label);
    filesystem_options_box_layout->addWidget(m_fs_specific_edit);

    toggleAdditionalOptions(true);

    connect(ext2_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));
    
    connect(ext3_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));
    
    connect(reiserfs3_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));

    connect(reiserfs4_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));
    
    return options_tab;
}

QWidget* MountDialog::mountPointBox()
{
    if( hasFstabEntry(m_device_to_mount) )
	m_mount_point = getFstabEntry(m_device_to_mount);
    else
	m_mount_point = "";

    QGroupBox *mount_point_box = new QGroupBox( i18n("Mount point") );
    QHBoxLayout *mount_point_layout = new QHBoxLayout();
    mount_point_box->setLayout(mount_point_layout);
    
    m_mount_point_edit = new KLineEdit(m_mount_point);

    if( m_mount_point_edit->text() == "" )
    	(button(KDialog::Ok))->setEnabled(false);

    mount_point_layout->addWidget(m_mount_point_edit);
    
    QPushButton *browse_button = new QPushButton( i18n("Browse"), this);
    mount_point_layout->addWidget(browse_button);

    connect(m_mount_point_edit, SIGNAL( textChanged(const QString) ), 
	    this, SLOT( toggleOKButton(const QString) ));

    connect(browse_button, SIGNAL( clicked(bool) ), 
	    this, SLOT( selectMountPoint(bool) ));

    return mount_point_box;
}

void MountDialog::selectMountPoint(bool)
{
    char device[BUFF_LEN];

    strncpy(device, m_device_to_mount.toAscii().data(), BUFF_LEN);
    
    if(m_mount_point != ""){
	const KUrl url( "file://" + QString(m_mount_point) );
	const QString filter = "*";
	KFileDialog dialog( url, filter, 0 );
	dialog.setModal(true);
	dialog.setMode(KFile::Directory);
	dialog.exec();
	m_mount_point = dialog.selectedFile();
    }
    else{
	const KUrl url( "file:///" );
	const QString filter = "*";
	KFileDialog dialog( url, filter, 0 );
	dialog.setModal(true);
	dialog.setMode(KFile::Directory);
	dialog.exec();
	m_mount_point = dialog.selectedFile();
    }
    
    if( m_mount_point.length() > 1 && m_mount_point.endsWith("/") )
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
    else
	m_filesystem_type = m_filesystem_edit->text();

    m_mount_point = m_mount_point_edit->text();

    if( m_mount_point.length() > 1 && m_mount_point.endsWith("/") )  // remove trailing slash
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

    if( acl_check->isChecked() )
	additional_options.append("acl");

    if( user_xattr_check->isChecked() )
	additional_options.append("user_xattr");

    if( m_fs_specific_edit->text().trimmed() != "" )
	additional_options.append( (m_fs_specific_edit->text()).trimmed() );
    
    all_options = standard_options.join(",");

    if( additional_options.size() ){
	all_options.append(",");
	all_options.append( additional_options.join(",") );
    }

    int error = mount( m_device_to_mount.toAscii().data(),
		       m_mount_point.toAscii().data(),
		       m_filesystem_type.toAscii().data(),
		       options, 
		       ( additional_options.join(",") ).toAscii().data() );

    if( !error )
	addMountEntry( m_device_to_mount, m_mount_point, m_filesystem_type, all_options, 0, 0);
    else
	KMessageBox::error(0, QString("Error number: %1  %2").arg(errno).arg(strerror(errno)));

}

void MountDialog::toggleOKButton()
{
    if ( (specify_button->isChecked() && m_filesystem_edit->text() == "" ) || m_mount_point_edit->text() == "")
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

    if( ext2_button->isChecked() || ext3_button->isChecked() || 
	reiserfs3_button->isChecked() || reiserfs4_button->isChecked() ){

	acl_check->setEnabled(true);
	user_xattr_check->setEnabled(true);
    }
    else{
	acl_check->setEnabled(false);
	acl_check->setChecked(false);
	user_xattr_check->setEnabled(false);
	user_xattr_check->setChecked(false);
    }
}

bool mount_filesystem(LogVol *volumeToMount)
{
    MountDialog dialog(volumeToMount->getMapperPath(), volumeToMount->getFilesystem(), volumeToMount->isWritable());
    dialog.exec();

    if (dialog.result() == QDialog::Accepted)
	return true;
    else
	return false;
}

bool mount_filesystem(StoragePartition *partition)
{
    QString device = partition->getName();
    QString filesystem = partition->getFileSystem();
    bool writable = partition->isWritable();

    MountDialog dialog(device, filesystem, writable);
    dialog.exec();

    if (dialog.result() == QDialog::Accepted)
	return true;
    else
	return false;
}
