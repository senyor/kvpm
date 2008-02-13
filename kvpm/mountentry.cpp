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


#include <mntent.h>
#include <fstab.h>
#include <string.h>
#include <stdio.h>
#include <QtGui>
#include "logvol.h"
#include "volgroup.h"
#include "mountentry.h"

const int BUFF_LEN = 2000;   // Enough?

/* Adds an entry into the mount table file, usually /etc/mtab.
   It returns 0 on success and 1 on failure  */

int addMountEntry(QString Device, QString MountPoint, QString Type, 
		   QString Options, int DumpFreq, int Pass)
{
    char mount_table[] = _PATH_MOUNTED;
    char device[BUFF_LEN];
    char mount_point[BUFF_LEN];
    char type[BUFF_LEN];
    char options[BUFF_LEN];

    strncpy(device,      Device.toAscii().data(),     BUFF_LEN);
    strncpy(mount_point, MountPoint.toAscii().data(), BUFF_LEN);
    strncpy(type,        Type.toAscii().data(),       BUFF_LEN);
    strncpy(options,     Options.toAscii().data(),    BUFF_LEN);
    
    const struct mntent mount_entry  = { device, mount_point, type, options, DumpFreq, Pass };
    
    FILE *fp = setmntent(mount_table, "a");
    if(fp){
	if( addmntent(fp, &mount_entry) ){
	    endmntent(fp);
	    return 1;
	}
	else{
	    endmntent(fp);
	    return 0;
	}
    }
    return 1;
}


/* 
   Add a comma separated list of mount options to an existing entry
   in the /etc/mtab file. 
*/

bool addMountEntryOptions(QString MountPoint, QString NewOptions)
{
    QString options, type, device;
    int dump_freq, pass;
    
    mntent *mount_entry;
    mntent *temp_entry;
    
    const char *mount_table_old = _PATH_MOUNTED;
    
    FILE *fp_old = setmntent(mount_table_old, "r");

    while( (mount_entry = getmntent(fp_old)) ){
	temp_entry = copyMountEntry(mount_entry);
	if( QString(temp_entry->mnt_dir) == MountPoint ){

	    options = temp_entry->mnt_opts;
	    options = options.simplified();

	    if( NewOptions.startsWith("," ) )
		options.append(NewOptions);
	    else
		options.append( "," + NewOptions);

	    type = temp_entry->mnt_type;
	    device = temp_entry->mnt_fsname;
	    dump_freq = temp_entry->mnt_freq;
	    pass = temp_entry->mnt_passno;
	    
	    endmntent(fp_old);
	    removeMountEntry(MountPoint);
	    addMountEntry(device, MountPoint, type, options, dump_freq, pass);

	    return TRUE;
	}
    }
    endmntent(fp_old);

    return FALSE;
}


bool removeMountEntry(QString MountPoint)
{
    QList<mntent *> mount_entry_list;
    mntent *mount_entry;
    mntent *temp_entry;
    
    const char *mount_table_old = _PATH_MOUNTED;
    const char *mount_table_new = "/etc/mtab.new";
    
    FILE *fp_old = setmntent(mount_table_old, "r");

    while( (mount_entry = getmntent(fp_old)) ){
	temp_entry = copyMountEntry(mount_entry);
	if( QString(temp_entry->mnt_dir) != MountPoint )
	    mount_entry_list.append(temp_entry);
    }
    endmntent(fp_old);

    FILE *fp_new = setmntent(mount_table_new, "w");
    for(int x = 0; x < mount_entry_list.size(); x++){
	const mntent *entry = mount_entry_list[x];
	addmntent(fp_new, entry);
    }
    endmntent(fp_new);

    rename("/etc/mtab.new", _PATH_MOUNTED);
    return TRUE;
}
	    
/* here we compare the complete path to a logical volume
   to a series of entries in _PATH_MOUNTED (probably "/etc/mtab") 
   to see if any of them match. Returns FALSE on error*/

bool hasMountEntry(QString Device)
{
    QString name_entry;        // returned entry to compare to

    const char mount_table[] = _PATH_MOUNTED;
    mntent *mount_entry;
    
    FILE *fp = setmntent(mount_table, "r");
    if(fp){
	while( (mount_entry = getmntent(fp)) ){
	    name_entry = QString( mount_entry->mnt_fsname );
	    if(name_entry == Device){
		endmntent(fp);
		return TRUE;
	    }
	}
	endmntent(fp);
	return FALSE;
    }
    return FALSE;
}


/* This function looks at the mount table and returns the
   filesystem's mount point if the device is has an entry
   in the table. It returns NULL on failure */

QString getMountEntry(QString Device)
{
    const char mount_table[] = _PATH_MOUNTED;
    mntent *mount_entry;
    QString name_entry;                // returned entry to compare to
    QString mount_dir;
    
    FILE *fp = setmntent(mount_table, "r");
    if(fp){
	while( (mount_entry = getmntent(fp)) ){
	    name_entry = QString( mount_entry->mnt_fsname );
	    if(name_entry == Device){
		mount_dir = QString( mount_entry->mnt_dir );
		endmntent(fp);
		return mount_dir;
	    }
	}
	endmntent(fp);
	return NULL;
    }
    return NULL;
}

bool hasFstabEntry(QString Device)
{
    QString name_entry;                // returned entry to compare to

    const char mount_table[] = _PATH_FSTAB;
    mntent *mount_entry;

    FILE *fp = setmntent(mount_table, "r");
    if(fp){
	while( (mount_entry = getmntent(fp)) ){
	    name_entry = QString( mount_entry->mnt_fsname );
	    if(name_entry == Device){
		endmntent(fp);
		return TRUE;
	    }
	}
	endmntent(fp);
	return FALSE;
    }
    return FALSE;
}

QString getFstabEntry(QString Device)
{
    QString mount_table = _PATH_FSTAB;
    QString name_entry;                // returned entry to compare to
    QString mount_dir;

    FILE *fp;
    mntent *mount_entry;
    
    fp = setmntent(mount_table.toAscii(), "r");

    while( (mount_entry = getmntent(fp)) ){
	name_entry = QByteArray( mount_entry->mnt_fsname );
	if(name_entry == Device){
	    mount_dir = QByteArray( mount_entry->mnt_dir );
	    endmntent(fp);
	    return mount_dir;
	}
    }
    endmntent(fp);
    return NULL;
}

mntent *copyMountEntry(mntent *MountEntry)
{
    mntent *new_entry = new mntent;

    new_entry->mnt_fsname = new char[BUFF_LEN]; 
    new_entry->mnt_dir    = new char[BUFF_LEN];
    new_entry->mnt_type   = new char[BUFF_LEN];
    new_entry->mnt_opts   = new char[BUFF_LEN];
    
    strncpy(new_entry->mnt_fsname, MountEntry->mnt_fsname, BUFF_LEN);
    strncpy(new_entry->mnt_dir,    MountEntry->mnt_dir,    BUFF_LEN);
    strncpy(new_entry->mnt_type,   MountEntry->mnt_type,   BUFF_LEN);
    strncpy(new_entry->mnt_opts,   MountEntry->mnt_opts,   BUFF_LEN);
    
    new_entry->mnt_freq   = MountEntry->mnt_freq;
    new_entry->mnt_passno = MountEntry->mnt_passno;
    
    return new_entry;
}

