/*
 *
 * 
 * Copyright (C) 2008, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include "mountentry.h"

#include <mntent.h>
#include <fstab.h>
#include <string.h>
#include <stdio.h>

#include <QtGui>

#include <kde_file.h>

#include "logvol.h"
#include "storagepartition.h"
#include "volgroup.h"


const int BUFF_LEN = 2000;   // Enough?

mntent *copyMountEntry(mntent *mountEntry);

mntent *buildMountEntry(QString device, QString mountPoint, QString type, 
			QString options, int dumpFreq, int pass);



// Adds an entry into the mount table file, usually /etc/mtab.

bool addMountEntry(QString device, QString mountPoint, QString type, QString options, int dumpFreq, int pass)
{
    QByteArray device_array      = device.toLocal8Bit();
    QByteArray mount_point_array = mountPoint.toLocal8Bit();
    QByteArray type_array        = type.toLocal8Bit();
    QByteArray options_array     = options.toLocal8Bit();

    const struct mntent mount_entry  = { device_array.data(), 
					 mount_point_array.data(), 
					 type_array.data(), 
					 options_array.data(), 
					 dumpFreq, 
					 pass };
    
    FILE *fp = setmntent(_PATH_MOUNTED, "a");

    if(fp){
	if( addmntent(fp, &mount_entry) ){
	    endmntent(fp);
	    return false;
	}
	else{              // success
	    endmntent(fp);
	    return true;
	}
    }
    else{
	return false;
    }
}


/* This function generates an mntent structure from its parameters
and returns it  */

mntent* buildMountEntry(QString device, QString mountPoint, QString type, QString options, int dumpFreq, int pass)
{
    QByteArray device_array      = device.toLocal8Bit();
    QByteArray mount_point_array = mountPoint.toLocal8Bit();
    QByteArray type_array        = type.toLocal8Bit();
    QByteArray options_array     = options.toLocal8Bit();

    mntent *mount_entry = new mntent;
    
    mount_entry->mnt_fsname = device_array.data();
    mount_entry->mnt_dir    = mount_point_array.data();
    mount_entry->mnt_type   = type_array.data();
    mount_entry->mnt_opts   = options_array.data();
    mount_entry->mnt_freq   = dumpFreq;
    mount_entry->mnt_passno = pass;

    return mount_entry;
}

bool removeMountEntry(QString mountPoint)
{
    QList<mntent *> mount_entry_list;
    mntent *mount_entry;
    mntent *temp_entry;
    QByteArray new_path(_PATH_MOUNTED);
    new_path.append(".new");     

    const char *mount_table_old = _PATH_MOUNTED;
    const char *mount_table_new = new_path.data();
    
    FILE *fp_old = setmntent(mount_table_old, "r");


/* Multiple devices can be mounted on a directory but only the
   last one mounted will be unmounted at one time. So only the
   last mtab entry gets deleted upon unmounting  */

    while( (mount_entry = getmntent(fp_old)) ){
	temp_entry = copyMountEntry(mount_entry);
	mount_entry_list.append(temp_entry);
    }
    for( int x = mount_entry_list.size() - 1; x >= 0; x--){
        if( QString( (mount_entry_list[x])->mnt_dir ) == mountPoint ){
	    mount_entry_list.removeAt(x);
	    break;
	}
    }
    endmntent(fp_old);

    FILE *fp_new = setmntent(mount_table_new, "w");
    for(int x = 0; x < mount_entry_list.size(); x++){
	const mntent *entry = mount_entry_list[x];
	addmntent(fp_new, entry);
    }
    endmntent(fp_new);

    KDE_rename(mount_table_new, mount_table_old);
    return true;
}
	    

mntent *copyMountEntry(mntent *mountEntry)
{
    mntent *new_entry = new mntent;

    new_entry->mnt_fsname = new char[BUFF_LEN]; 
    new_entry->mnt_dir    = new char[BUFF_LEN];
    new_entry->mnt_type   = new char[BUFF_LEN];
    new_entry->mnt_opts   = new char[BUFF_LEN];
    
    strncpy(new_entry->mnt_fsname, mountEntry->mnt_fsname, BUFF_LEN);
    strncpy(new_entry->mnt_dir,    mountEntry->mnt_dir,    BUFF_LEN);
    strncpy(new_entry->mnt_type,   mountEntry->mnt_type,   BUFF_LEN);
    strncpy(new_entry->mnt_opts,   mountEntry->mnt_opts,   BUFF_LEN);
    
    new_entry->mnt_freq   = mountEntry->mnt_freq;
    new_entry->mnt_passno = mountEntry->mnt_passno;
    
    return new_entry;
}

bool rename_mount_entries(QString oldName, QString newName)
{
    QList<mntent *> mount_entry_list;
    mntent *mount_entry;
    mntent *temp_entry;
    
    QByteArray new_path(_PATH_MOUNTED);
    new_path.append(".new");     

    const char *mount_table_old = _PATH_MOUNTED;
    const char *mount_table_new = new_path.data();
    
    FILE *fp_old = setmntent(mount_table_old, "r");

    while( (mount_entry = getmntent(fp_old)) ){
	temp_entry = copyMountEntry(mount_entry);

	if( QString(temp_entry->mnt_fsname) != oldName ){
	    mount_entry_list.append(temp_entry);
	}
	else{
	    temp_entry = buildMountEntry( newName, 
					  temp_entry->mnt_dir,
					  temp_entry->mnt_type,	 
					  temp_entry->mnt_opts,
					  temp_entry->mnt_freq,
					  temp_entry->mnt_passno );
	    
	    mount_entry_list.append(temp_entry);
	}
    }
    
    endmntent(fp_old);

    FILE *fp_new = setmntent(mount_table_new, "w");
    for(int x = 0; x < mount_entry_list.size(); x++){
	const mntent *entry = mount_entry_list[x];
	addmntent(fp_new, entry);
    }
    endmntent(fp_new);

    KDE_rename(mount_table_new, mount_table_old);

    return true;
}

