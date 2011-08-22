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
#include <stdio.h>

#include <QtGui>

#include "logvol.h"
#include "mountentry.h"
#include "mountinfo.h"
#include "volgroup.h"


MountInformationList::MountInformationList()
{
    const char mount_table[] = _PATH_MOUNTED;
    mntent *mount_entry;
    
    FILE *fp = setmntent(mount_table, "r");

    if(fp){
	while( (mount_entry = getmntent(fp)) )
	    m_list.append( new MountInformation(mount_entry) );

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
