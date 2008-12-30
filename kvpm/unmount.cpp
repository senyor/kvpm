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


#include <sys/mount.h>
#include <errno.h>
#include <string.h>

#include <KMessageBox>
#include <KLocale>
#include <QtGui>

#include "logvol.h"
#include "mountentry.h"
#include "nomungecheck.h"
#include "storagepartition.h"
#include "unmount.h"


/* The unmount dialog is only needed for unmounting devices that have
   more than one mount point. The user can select which one to unmount.
   KMessageBox type dialogs are used otherwise */


UnmountDialog::UnmountDialog(QString device, QStringList mountPoints, QWidget *parent) : 
    KDialog(parent),
    m_mount_points(mountPoints)
{

    NoMungeCheck *temp_check;
    QString unmount_message;
    
    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);

    QVBoxLayout *layout    = new QVBoxLayout();
    dialog_body->setLayout(layout);

    unmount_message = i18n( "<b>%1</b> is mounted at multiple locatations. "
			    "Check the boxes for any you "
			    "wish to unmount." ).arg(device);

    QLabel *unmount_message_label = new QLabel();
    unmount_message_label->setText(unmount_message);
    unmount_message_label->setWordWrap(true);
    layout->addWidget(unmount_message_label);

    QGroupBox *mount_group = new QGroupBox( i18n("Mount points") );
    QVBoxLayout *mount_group_layout = new QVBoxLayout();
    mount_group->setLayout(mount_group_layout);
    layout->addWidget(mount_group);
    
    for(int x = 0; x < m_mount_points.size(); x++){
	temp_check = new NoMungeCheck( m_mount_points[x] );
	mount_group_layout->addWidget(temp_check);
	m_check_list.append(temp_check);
    }

    connect(this, SIGNAL( accepted() ), 
	    this, SLOT( unmountFilesystems() ));

}

void UnmountDialog::unmountFilesystems()
{
    QString mount_point, error_string, error_message;
    
    for(int x = 0; x < m_check_list.size(); x++){
	if( m_check_list[x]->isChecked() ) {
	    
	    mount_point = m_check_list[x]->getUnmungedText();
	    if( umount2( mount_point.toAscii().data() , 0) ){
		error_string =  strerror( errno );

		error_message = i18n("Unmounting %1 failed with error number: %2 "
				     "%3").arg(mount_point).arg(errno).arg(error_string);

		KMessageBox::error(0, error_message);
	    }
	    else
		removeMountEntry(mount_point);
	} 
    }
}

bool unmount_filesystem(LogVol *logicalVolume)
{
    QStringList mount_points = logicalVolume->getMountPoints();
    QString name = logicalVolume->getName();
    QString vg_name = logicalVolume->getVolumeGroupName();
    QString unused_message;
    QString unmount_point;                 // mount point to unmount

    if( logicalVolume->isMounted() ){

        unused_message = i18n("The volume <b>%1</b> is mounted on <b>%2</b> "
			      "Do you want to unmount it?" ).arg(name).arg(mount_points[0]);

        if( mount_points.size() == 1 ){
            if( KMessageBox::questionYesNo(0, unused_message) == 3 ){     // 3 is "yes"
                unmount_point = mount_points[0];
                return( unmount_filesystem( unmount_point ) );
            }
            else
                return false;
        }
        else{
            UnmountDialog dialog("/dev/mapper/" + vg_name + "-" + name  ,mount_points);
            dialog.exec();
            return true;
        }
    }

    KMessageBox::error(0, i18n("The volume: <b>%1</b> does not seem to be mounted").arg(name) );
    return false;
}

bool unmount_filesystem(StoragePartition *partition)
{
    QStringList mount_points = partition->getMountPoints();
    QString path = partition->getPartitionPath();

    QString unused_message;
    QString unmount_point;                 // mount point to unmount

    if( partition->isMounted() ){
    
	unused_message = i18n("The partition <b>%1</b> is mounted on <b>%2</b> "
			      "Do you want to unmount it?").arg(path).arg(mount_points[0]);

	if( mount_points.size() == 1 ){
	    if( KMessageBox::questionYesNo(0, unused_message) == 3 ){     // 3 is "yes"
		unmount_point = mount_points[0];
		return( unmount_filesystem( unmount_point ) );
	    }
	    else
		return false;
	}
	else{
	    UnmountDialog dialog(path, mount_points);
	    dialog.exec();
	    return true;
	}
    }
    KMessageBox::error(0, i18n("The partition: <b>%1</b> does not seem to be mounted").arg(path)); 
    return false;
}


/* This function calls umount2 for the actual unmounting if the dialog
   asks for it. It also removes the entry from "mtab" on success */

bool unmount_filesystem(const QString mountPoint)
{
    QString error_string, error_message;

    const char *mount_point = mountPoint.toAscii().data();

    if( umount2(mount_point, 0) ){
	error_string =  strerror( errno );
	
	error_message = i18n("Unmounting %1 failed with error number: %2 "
			     "%3").arg(mountPoint).arg(errno).arg(error_string);
	
	KMessageBox::error(0, error_message);
	return false;
    }
    else{
	removeMountEntry(mountPoint);
	return true;
    }
}
