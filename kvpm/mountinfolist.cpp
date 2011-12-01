/*
 *
 * 
 * Copyright (C) 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include "mountinfolist.h"


#include <mntent.h>
#include <fstab.h>
#include <stdio.h>

#include <QtGui>

#include "logvol.h"
#include "mountentry.h"
#include "mountinfo.h"
#include "storagepartition.h"
#include "volgroup.h"


MountInformationList::MountInformationList()
{
    mntent *mount_entry;
    FILE *fp = setmntent(_PATH_MOUNTED, "r");

    if(fp){
	while( (mount_entry = getmntent(fp)) )
	    m_list.append( new MountInformation(mount_entry) );

	endmntent(fp);
    }

    fp = setmntent(_PATH_FSTAB, "r");

    if(fp){
	while( (mount_entry = getmntent(fp)) )
	    m_fstab_list.append( new MountInformation(mount_entry) );

	endmntent(fp);
    }
}

MountInformationList::~MountInformationList()
{
    for(int x = 0; x < m_list.size(); x++)
	delete (m_list[x]);
}

QList<MountInformation *> MountInformationList::getMountInformation(QString deviceName)
{
    QList<MountInformation *> device_mounts;
    
    for(int x = m_list.size() - 1; x >= 0; x--){
	if( deviceName == m_list[x]->getDeviceName() )
	    device_mounts.append( m_list.takeAt(x) );
    }

    return device_mounts;
}

QString MountInformationList::getFstabMountPoint(LogVol *const lv)
{
    return getFstabMountPoint( lv->getMapperPath(), lv->getFilesystemLabel(), lv->getFilesystemUuid() );
}

QString MountInformationList::getFstabMountPoint(StoragePartition *const partition)
{
    return getFstabMountPoint( partition->getName(), partition->getFilesystemLabel(), partition->getFilesystemUuid() );
}

QString MountInformationList::getFstabMountPoint(const QString name, const QString label, const QString uuid)
{
    QString entry_name;
    MountInformation *entry;

    QListIterator<MountInformation *> entry_itr(m_fstab_list);
    while( entry_itr.hasNext() ){
        entry = entry_itr.next();
        entry_name = entry->getDeviceName();

        if( entry_name.startsWith("UUID=", Qt::CaseInsensitive) ){
            entry_name = entry_name.remove(0, 5);
            if( entry_name == uuid )
                return entry->getMountPoint();
        }
        else if( entry_name.startsWith("LABEL=", Qt::CaseInsensitive) ){
            entry_name = entry_name.remove(0, 6);
            if( entry_name == label )
                return entry->getMountPoint();
        }
        else if( entry_name == name )
            return entry->getMountPoint();
    }

    return QString();
}
