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


/*    Add a comma separated list of mount options to an existing entry
      in the /etc/mtab file.  */

bool addMountEntryOptions(QString mountPoint, QString newOptions)
{
    QString options, type, device;
    int dump_freq, pass;
    
    mntent *mount_entry;
    mntent *temp_entry;
    
    const char *mount_table_old = _PATH_MOUNTED;
    
    FILE *fp_old = setmntent(mount_table_old, "r");

    while( (mount_entry = getmntent(fp_old)) ){
	temp_entry = copyMountEntry(mount_entry);
	if( QString(temp_entry->mnt_dir) == mountPoint ){

	    options = temp_entry->mnt_opts;
	    options = options.simplified();

	    if( newOptions.startsWith(',' ) )
		options.append(newOptions);
	    else
		options.append( ',' + newOptions);

	    type = temp_entry->mnt_type;
	    device = temp_entry->mnt_fsname;
	    dump_freq = temp_entry->mnt_freq;
	    pass = temp_entry->mnt_passno;
	    
	    endmntent(fp_old);
	    removeMountEntry(mountPoint);
	    addMountEntry(device, mountPoint, type, options, dump_freq, pass);

	    return true;
	}
    }
    endmntent(fp_old);

    return false;
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
	    
/* here we compare the complete path to a logical volume
   to a series of entries in _PATH_MOUNTED (probably "/etc/mtab") 
   to see if any of them match. Returns false on error*/

bool hasMountEntry(QString device)
{
    QString name_entry;        // returned entry to compare to
    mntent *mount_entry;
    
    FILE *fp = setmntent(_PATH_MOUNTED, "r");
    if(fp){
	while( (mount_entry = getmntent(fp)) ){
	    name_entry = QString( mount_entry->mnt_fsname );
	    if(name_entry == device){
		endmntent(fp);
		return true;
	    }
	}
	endmntent(fp);
	return false;
    }
    else{
	return false;
    }
}


/* This function looks at the mount table and returns the
   filesystem's mount point if the device is has an entry
   in the table. It returns NULL on failure */


QStringList getMountedDevices(QString mountPoint)
{
    mntent *mount_entry;
    QStringList mounted_devices;

    FILE *fp = setmntent(_PATH_MOUNTED, "r");
    if(fp){
	while( (mount_entry = getmntent(fp)) ){

	    if( QString( mount_entry->mnt_dir ) == mountPoint ){
	       mounted_devices.append( QString( mount_entry->mnt_fsname ) );

	    }
	}
	endmntent(fp);
    }

    return mounted_devices;
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

QString getFstabEntry(LogVol *const lv)
{
    const QString lv_name  = lv->getMapperPath();
    const QString fs_label = lv->getFilesystemLabel();
    const QString fs_uuid  = lv->getFilesystemUuid();

    QString entry;                // returned entry to compare to
    QString mount_dir;
    mntent *mount_entry;

    FILE *const fp = setmntent(_PATH_FSTAB, "r");

    while( (mount_entry = getmntent(fp)) ){
	entry = QByteArray( mount_entry->mnt_fsname );
        entry = entry.trimmed();

        if( entry.startsWith("UUID=", Qt::CaseInsensitive) )
            entry = entry.remove(0, 5);
        else if( entry.startsWith("LABEL=", Qt::CaseInsensitive) )
            entry = entry.remove(0, 6);

	if(entry == lv_name || entry == fs_uuid || entry == fs_label){
	    mount_dir = QByteArray( mount_entry->mnt_dir );
	    endmntent(fp);
	    return mount_dir;
	}
    }
    endmntent(fp);

    return QString();
}

QString getFstabEntry(StoragePartition *const partition)
{
    const QString part_name  = partition->getName();
    const QString fs_label   = partition->getFilesystemLabel();
    const QString fs_uuid    = partition->getFilesystemUuid();

    QString entry;                // returned entry to compare to
    QString mount_dir;
    mntent *mount_entry;

    FILE *const fp = setmntent(_PATH_FSTAB, "r");

    while( (mount_entry = getmntent(fp)) ){
	entry = QByteArray( mount_entry->mnt_fsname );

        if( entry.startsWith("UUID=", Qt::CaseInsensitive) )
            entry = entry.remove(0, 5);
        else if( entry.startsWith("LABEL=", Qt::CaseInsensitive) )
            entry = entry.remove(0, 6);

	if(entry == part_name || entry == fs_uuid || entry == fs_label){
	    mount_dir = QByteArray( mount_entry->mnt_dir );
	    endmntent(fp);
	    return mount_dir;
	}
    }
    endmntent(fp);

    return QString();
}
