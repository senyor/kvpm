/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Klvm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <sys/mount.h>
#include <errno.h>
#include <string.h>
#include <KTabWidget>
#include <KFileDialog>
#include <KPushButton>
#include <KMessageBox>
#include <KUrl>
#include <QtGui>
#include "logvol.h"
#include "mount.h"
#include "mountentry.h"
#include "storagepartition.h"


const int BUFF_LEN = 2000;   // Enough?


MountDialog::MountDialog(QString DeviceToMount, QString Filesystem, QWidget *parent) : KDialog(parent)
{
    device_to_mount = DeviceToMount;
    filesystem_type = Filesystem;
    
    KTabWidget *dialog_body = new KTabWidget(this);
    setMainWidget(dialog_body);

    QWidget *main_tab    = new QWidget(this);
    QWidget *options_tab = new QWidget(this);
    dialog_body->addTab(main_tab,    "Main");
    dialog_body->addTab(options_tab, "Options");
    main_layout    = new QVBoxLayout();
    options_layout = new QVBoxLayout();
    main_tab->setLayout(main_layout);
    options_tab->setLayout(options_layout);
    
    setupFilesystemBox();
    setupOptionsTab();
    setupMountPointBox();

    connect(this, SIGNAL( accepted() ), this, SLOT( mountFilesystem() ));
    
}

void MountDialog::setupFilesystemBox()
{
    QGroupBox   *filesystem_box = new QGroupBox("Filesystem type", this);
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
    reiserfs3_button = new QRadioButton("Reiserfs Ver. 3", this);
    reiserfs4_button = new QRadioButton("Reiserfs Ver. 4", this);
    xfs_button       = new QRadioButton("Xfs", this);
    jfs_button       = new QRadioButton("Jfs", this);
    vfat_button      = new QRadioButton("vfat", this);
    udf_button       = new QRadioButton("udf", this);
    iso9660_button   = new QRadioButton("iso9660", this);
    hfs_button       = new QRadioButton("hfs", this);
    
    specify_button   = new QRadioButton("Specify other:", this);
    if( filesystem_type == "ext2" )
	ext2_button->setChecked(TRUE);
    else if( filesystem_type == "ext3" )
	ext3_button->setChecked(TRUE);
    else if( filesystem_type == "reiserfs" )
	reiserfs3_button->setChecked(TRUE);
    else if( filesystem_type == "reiser4" )
	reiserfs4_button->setChecked(TRUE);
    else if( filesystem_type == "xfs" )
	xfs_button->setChecked(TRUE);
    else if( filesystem_type == "jfs" )
	jfs_button->setChecked(TRUE);
    else if( filesystem_type == "vfat" )
	vfat_button->setChecked(TRUE);
    else if( filesystem_type == "udf" )
	udf_button->setChecked(TRUE);
    else if( filesystem_type == "iso9660" )
	iso9660_button->setChecked(TRUE);
    else if( filesystem_type == "hfs" )
	hfs_button->setChecked(TRUE);
    else
	specify_button->setChecked(TRUE);
    filesystem_edit = new KLineEdit(filesystem_type);

    if( specify_button->isChecked() && filesystem_edit->text() == "" )
	(button(KDialog::Ok))->setEnabled(FALSE);
    
    layout_left->addWidget(ext2_button);
    layout_left->addWidget(ext3_button);
    layout_left->addWidget(vfat_button);
    layout_center->addWidget(reiserfs3_button);
    layout_center->addWidget(reiserfs4_button);
    layout_center->addWidget(xfs_button);
    layout_center->addWidget(jfs_button);
    layout_right->addWidget(udf_button);
    layout_right->addWidget(iso9660_button);
    layout_right->addWidget(hfs_button);
    
    lower_layout->addWidget(specify_button);
    lower_layout->addWidget(filesystem_edit);
    main_layout->addWidget(filesystem_box);

    connect(filesystem_edit, SIGNAL( textEdited(const QString) ), 
	    this, SLOT( toggleOKButton(const QString) ));

    connect(specify_button, SIGNAL( toggled(bool) ), 
	    this, SLOT( toggleOKButton(bool) ));
}

void MountDialog::setupOptionsTab()
{
    QGroupBox *common_options_box = new QGroupBox("Common mount options", this);
    QVBoxLayout *layout_left      = new QVBoxLayout();
    QVBoxLayout *layout_right     = new QVBoxLayout();
    QHBoxLayout *common_options_layout   = new QHBoxLayout();
    common_options_layout->addLayout(layout_left);
    common_options_layout->addLayout(layout_right);
    common_options_box->setLayout(common_options_layout);

    sync_check     = new QCheckBox("sync");
    dirsync_check  = new QCheckBox("dirsync");
    dirsync_check->setEnabled(FALSE);
    rw_check       = new QCheckBox("rw");
    suid_check     = new QCheckBox("suid");
    dev_check      = new QCheckBox("dev");
    exec_check     = new QCheckBox("exec");
    mand_check     = new QCheckBox("mand");
    acl_check      = new QCheckBox("acl");
    user_xattr_check = new QCheckBox("user xattr");
    
    rw_check->setChecked(TRUE);
    suid_check->setChecked(TRUE);
    dev_check->setChecked(TRUE);
    exec_check->setChecked(TRUE);
    
    sync_check->setToolTip("Always use synchronous I/O");
    rw_check->setToolTip("Allow writing in addition to reading");
    suid_check->setToolTip("Allow the suid bit to have effect");
    dev_check->setToolTip("Allow the use of block and special devices");
    exec_check->setToolTip("Allow the execution of binary files");
    mand_check->setToolTip("Allow manditory file locks");
    acl_check->setToolTip("Allow use of access control lists");
    
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
    QGroupBox *atime_box = new QGroupBox("Update atime", this);
    QVBoxLayout *atime_layout = new QVBoxLayout();
    atime_box->setLayout(atime_layout);
    options_atime_layout->addWidget(common_options_box);

    filesystem_journal_box = new QGroupBox("Journaling");
    QVBoxLayout *journaling_layout = new QVBoxLayout;
    data_journal_button = new QRadioButton("data=journal");
    data_ordered_button = new QRadioButton("data=ordered");
    data_ordered_button->setChecked(TRUE);
    data_writeback_button = new QRadioButton("data=writeback");
    filesystem_journal_box->setLayout(journaling_layout);
    journaling_layout->addWidget(data_journal_button);
    journaling_layout->addWidget(data_ordered_button);
    journaling_layout->addWidget(data_writeback_button);
    options_atime_layout->addWidget(filesystem_journal_box);

    atime_button = new QRadioButton("atime");
    atime_button->setChecked(TRUE);
    noatime_button = new QRadioButton("noatime");
    nodiratime_button = new QRadioButton("nodiratime");
    relatime_button = new QRadioButton("relatime");
    relatime_button->setEnabled(FALSE);
    atime_layout->addWidget(atime_button);
    atime_layout->addWidget(noatime_button);
    atime_layout->addWidget(nodiratime_button);
    atime_layout->addWidget(relatime_button);
    atime_button->setToolTip("Always update atime, this is the default");
    noatime_button->setToolTip("Do not update atime");
    nodiratime_button->setToolTip("Do not update atime for directory access");
    relatime_button->setToolTip("Access time is only updated if the previous access time was earlier than  the  current  modify  or  change time");
    
    options_atime_layout->addWidget(atime_box);

    QGroupBox *filesystem_options_box = new QGroupBox("Filesystem specific  mount options");
    fs_specific_edit = new KLineEdit();
    QLabel *additional_options_label = new QLabel("comma separated list of additional mount options");
    
    options_layout->addWidget(filesystem_options_box);
    QVBoxLayout *filesystem_options_box_layout = new QVBoxLayout();
    
    filesystem_options_box->setLayout(filesystem_options_box_layout);
    filesystem_options_box_layout->addWidget(additional_options_label);
    filesystem_options_box_layout->addWidget(fs_specific_edit);

    toggleAdditionalOptions(TRUE);

    connect(ext2_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));
    
    connect(ext3_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));
    
    connect(reiserfs3_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));

    connect(reiserfs4_button, SIGNAL( toggled(bool) ),
	    this, SLOT( toggleAdditionalOptions(bool)));
    
}

void MountDialog::setupMountPointBox()
{
    if( hasFstabEntry(device_to_mount) )
	mount_point = getFstabEntry(device_to_mount);
    else
	mount_point = "";

    QGroupBox *mount_point_box = new QGroupBox("Mount point");
    QHBoxLayout *mount_point_layout = new QHBoxLayout();
    mount_point_box->setLayout(mount_point_layout);
    
    mount_point_edit = new KLineEdit(mount_point);
    if( mount_point_edit->text() == "" )
    	(button(KDialog::Ok))->setEnabled(FALSE);

    mount_point_layout->addWidget(mount_point_edit);
    
    QPushButton *browse_button = new QPushButton("Browse", this);
    mount_point_layout->addWidget(browse_button);
    main_layout->addWidget(mount_point_box);

    connect(mount_point_edit, SIGNAL( textChanged(const QString) ), 
	    this, SLOT( toggleOKButton(const QString) ));

    connect(browse_button, SIGNAL( clicked(bool) ), 
	    this, SLOT( selectMountPoint(bool) ));
}

void MountDialog::selectMountPoint(bool)
{
    char device[BUFF_LEN];

    strncpy(device, device_to_mount.toAscii().data(), BUFF_LEN);
    
    if(mount_point != ""){
	const KUrl url( "file://" + QString(mount_point) );
	const QString filter = "*";
	KFileDialog dialog( url, filter, 0 );
	dialog.setModal(TRUE);
	dialog.setMode(KFile::Directory);
	dialog.exec();
	mount_point = dialog.selectedFile();
    }
    else{
	const KUrl url( "file:///" );
	const QString filter = "*";
	KFileDialog dialog( url, filter, 0 );
	dialog.setModal(TRUE);
	dialog.setMode(KFile::Directory);
	dialog.exec();
	mount_point = dialog.selectedFile();
    }
    
    if( mount_point.length() > 1 && mount_point.endsWith("/") )
	mount_point.chop(1);                           // remove trailing "/" character

    mount_point_edit->setText(mount_point);
}

void MountDialog::mountFilesystem()
{
    unsigned long options = 0;
    QStringList standard_options, additional_options;
    QString all_options; 
    
    if( ext2_button->isChecked() )
	filesystem_type = "ext2";
    else if( ext3_button->isChecked() )
	filesystem_type = "ext3";
    else if( reiserfs3_button->isChecked() )
	filesystem_type = "reiserfs";
    else if( reiserfs4_button->isChecked() )
	filesystem_type = "reiser4";
    else if( xfs_button->isChecked() )
	filesystem_type = "xfs";
    else if( jfs_button->isChecked() )
	filesystem_type = "jfs";
    else if( vfat_button->isChecked() )
	filesystem_type = "vfat";
    else
	filesystem_type = filesystem_edit->text();

    mount_point = mount_point_edit->text();
    if( mount_point.length() > 1 && mount_point.endsWith("/") )  // remove trailing slash
	mount_point.chop(1);

    if( sync_check->isChecked() ){
	standard_options.append("sync");
	options |= MS_SYNCHRONOUS;
    }
//    if( dirsync_check->isChecked() ){
//	standard_options.append("dirsync");
//	options |= MS_DIRSYNC;
//  }
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

    if( noatime_button->isChecked() ){
	standard_options.append("noatime");
	options |= MS_NOATIME;
    }
    else if( nodiratime_button->isChecked() ){
	standard_options.append("nodiratime");
	options |= MS_NODIRATIME;
    }
//    else if( relatime_button->isChecked() ){
//	standard_options.append("nodiratime");
//	options |= MS_RELATIME;
//  }

/* "data=ordered" is the default so we ignore that button */

    if( filesystem_journal_box->isEnabled() ){
	if( data_journal_button->isChecked() ) 
	    additional_options.append("data=journal");
	else if( data_writeback_button->isChecked() ) 
	    additional_options.append("data=writeback");
    }

    if( acl_check->isChecked() )
	additional_options.append("data=journal");
    if( user_xattr_check->isChecked() )
	additional_options.append("user_xattr");

    if( fs_specific_edit->text().trimmed() != "" )
	additional_options.append( (fs_specific_edit->text()).trimmed() );
    
    all_options = standard_options.join(",");
    if( additional_options.size() ){
	all_options.append(",");
	all_options.append( additional_options.join(",") );
    }

    int error = mount( device_to_mount.toAscii().data(),
		       mount_point.toAscii().data(),
		       filesystem_type.toAscii().data(),
		       options, 
		       ( additional_options.join(",") ).toAscii().data() );

    if( !error )
	addMountEntry( device_to_mount, mount_point, filesystem_type, all_options, 0, 0);
    else
	KMessageBox::error(0, QString("Error number: %1  %2").arg(errno).arg(strerror(errno)));
}

void MountDialog::toggleOKButton(const QString)
{
    if ( (specify_button->isChecked() && filesystem_edit->text() == "" ) ||
	 mount_point_edit->text() == "")

    	(button(KDialog::Ok))->setEnabled(FALSE);
    else
    	(button(KDialog::Ok))->setEnabled(TRUE);
}

void MountDialog::toggleOKButton(bool)
{
    if ( (specify_button->isChecked() && filesystem_edit->text() == "" ) ||
	 mount_point_edit->text() == "")

    	(button(KDialog::Ok))->setEnabled(FALSE);
    else
    	(button(KDialog::Ok))->setEnabled(TRUE);
}

void MountDialog::toggleAdditionalOptions(bool)
{
    if( ext3_button->isChecked() )
        filesystem_journal_box->setEnabled(TRUE);
    else
        filesystem_journal_box->setEnabled(FALSE);

    if( ext2_button->isChecked() || ext3_button->isChecked() || 
	reiserfs3_button->isChecked() || reiserfs4_button->isChecked() ){

	acl_check->setEnabled(TRUE);
	user_xattr_check->setEnabled(TRUE);
    }
    else{
	acl_check->setEnabled(FALSE);
	acl_check->setChecked(FALSE);
	user_xattr_check->setEnabled(FALSE);
	user_xattr_check->setChecked(FALSE);
    }
}

bool mount_filesystem(LogVol *VolumeToMount)
{
    const QString prefix = "/dev/mapper/";
    const QString lv_name = VolumeToMount->getName();
    const QString vg_name = VolumeToMount->getVolumeGroupName();

    MountDialog dialog(prefix + vg_name + "-" + lv_name, VolumeToMount->getFS());
    dialog.exec();
    if (dialog.result() == QDialog::Accepted)
	return TRUE;
    else
	return FALSE;
}

bool mount_filesystem(StoragePartition *Partition)
{
    QString device = Partition->getPartitionPath();
    QString filesystem = Partition->getFileSystem();
    
    MountDialog dialog(device, filesystem);
    dialog.exec();
    if (dialog.result() == QDialog::Accepted)
	return TRUE;
    else
	return FALSE;
}
