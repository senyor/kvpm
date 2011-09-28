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


#include "unmount.h"

#include <sys/mount.h>
#include <errno.h>
#include <string.h>

#include <KMessageBox>
#include <KLocale>
#include <QtGui>

#include "logvol.h"
#include "mountentry.h"
#include "misc.h"
#include "storagepartition.h"
#include "volgroup.h"


/* The unmount dialog is only needed for unmounting devices that have
   more than one mount point. The user can select which one to unmount.
   KMessageBox type dialogs are used otherwise */


UnmountDialog::UnmountDialog(QString device, QStringList mountPoints, 
			     QList<int> mountPosition, QWidget *parent) : 
    KDialog(parent),
    m_mount_points(mountPoints)
{

    NoMungeCheck *temp_check;
    bool checks_disabled = false;
    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);

    QVBoxLayout *layout    = new QVBoxLayout();
    dialog_body->setLayout(layout);

    QString unmount_message = i18n( "<b>%1</b> is mounted at multiple locatations. "
				    "Check the boxes for any you "
				    "wish to unmount.", device);

    QString position_message = i18n( "One or more unmount selections have been disabled. "
				     "Another device or volume is mounted over the same "
				     "mount point and must be unmounted first" );


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
	if( mountPosition[x] > 1 ){
      	    temp_check->setChecked(false);
	    temp_check->setEnabled(false);
	    checks_disabled = true;
	}
	mount_group_layout->addWidget(temp_check);
	m_check_list.append(temp_check);
    }
    if( checks_disabled ){
        QLabel *position_message_label = new QLabel();
	position_message_label->setText(position_message);
	position_message_label->setWordWrap(true);
	mount_group_layout->addWidget(position_message_label);
    }

    connect(this, SIGNAL( accepted() ), 
	    this, SLOT( unmountFilesystems() ));

}

void UnmountDialog::unmountFilesystems()
{
    QByteArray mount_point;
    QString error_string, error_message;

    for(int x = 0; x < m_check_list.size(); x++){
	if( m_check_list[x]->isChecked() ) {
	    
	    mount_point = m_check_list[x]->getUnmungedText().toLocal8Bit();
	    if( umount2( mount_point.data() , 0) ){
		error_string =  strerror( errno );

		error_message = i18n("Unmounting %1 failed with error number: %2 "
				     "%3", QString(mount_point), errno, error_string );

		KMessageBox::error(0, error_message);
	    }
	    else
		removeMountEntry(mount_point);
	} 
    }
}

bool unmount_filesystem(LogVol *logicalVolume)
{
    QStringList mount_points   = logicalVolume->getMountPoints();
    QList<int>  mount_position = logicalVolume->getMountPosition();

    QString name = logicalVolume->getName();
    QString vg_name = logicalVolume->getVG()->getName();

    QString unused_message = i18n("The volume <b>%1</b> is mounted on <b>%2</b> "
				  "Do you want to unmount it?", name, mount_points[0]);

    QString position_message = i18n("Another device or volume is mounted at the same "
				    "mount point over this one. It must be unmounted "
				    "before this one can be unmounted" );

    QString unmount_point;                 // mount point to unmount

    if( logicalVolume->isMounted() ){
        if( mount_points.size() == 1 ){
	    if( mount_position[0] < 2 )  {
                if( KMessageBox::questionYesNo(0, unused_message) == 3 ){     // 3 is "yes"
                    unmount_point = mount_points[0];
                    return( unmount_filesystem( unmount_point ) );
                }
		else
                    return false;
	    }
	    else{
	        KMessageBox::error(0, position_message);
	        return false;
	    }
        }
        else{
	    UnmountDialog dialog(logicalVolume->getMapperPath()  ,mount_points, mount_position);
	    if( dialog.exec() )
	      return true;
	    else
	      return false;
        }
    }

    KMessageBox::error(0, i18n("The volume: <b>%1</b> does not seem to be mounted", name) );
    return false;
}

bool unmount_filesystem(StoragePartition *partition)
{
    QList<int> mount_position = partition->getMountPosition();
    QStringList mount_points  = partition->getMountPoints();
    QString path = partition->getName();

    QString unused_message = i18n("The partition <b>%1</b> is mounted on <b>%2</b> "
				  "Do you want to unmount it?", path, mount_points[0]);

    QString position_message = i18n("Another device or volume is mounted at the same "
				    "mount point over this one. It must be unmounted "
				    "before this one can be unmounted" );

    QString unmount_point;                 // mount point to unmount

    if( partition->isMounted() ){
        if( mount_points.size() == 1 ){
	    if( mount_position[0] < 2 )  {
	        if( KMessageBox::questionYesNo(0, unused_message) == 3 ){     // 3 is "yes"
		  unmount_point = mount_points[0];
		  return( unmount_filesystem( unmount_point ) );
		}
		else
		    return false;
	    }
	    else{
	        KMessageBox::error(0, position_message);
		return false;
	    }
	}
	else{
  	    UnmountDialog dialog(path, mount_points, mount_position );

	    if( dialog.exec() )
	      return true;
	    else
	      return false;
	}
    }

    KMessageBox::error(0, i18n("The partition: <b>%1</b> does not seem to be mounted", path) ); 
    return false;
}


/* This function calls umount2 for the actual unmounting if the dialog
   asks for it. It also removes the entry from "mtab" on success */

bool unmount_filesystem(const QString mountPoint)
{
    QString error_string, error_message;
    QByteArray mount_point = mountPoint.toLocal8Bit();

    if( umount2(mount_point.data(), 0) ){
	error_string =  strerror( errno );
	
	error_message = i18n("Unmounting %1 failed with error number: %2 "
			     "%3", mountPoint, errno, error_string );
	
	KMessageBox::error(0, error_message);
	return false;
    }
    else{
	removeMountEntry(mountPoint);
	return true;
    }
}
