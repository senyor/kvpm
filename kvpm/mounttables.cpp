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


#include "mounttables.h"

#include <mntent.h>
#include <fstab.h>
#include <string.h>
#include <stdio.h>

#include <QtGui>

#include <kde_file.h>

#include "logvol.h"
#include "mountentry.h"
#include "storagepartition.h"
#include "volgroup.h"



const int BUFF_LEN = 2000;   // Enough?


mntent *buildMntent(const QString device, const QString mountPoint, const QString type, 
                    const QString options, const int dumpFreq, const int pass);

mntent *copyMntent(mntent *const entry);


MountTables::MountTables()
{
}

MountTables::~MountTables()
{
    for(int x = 0; x < m_list.size(); x++)
	delete (m_list[x]);

    for(int x = 0; x < m_fstab_list.size(); x++)
	delete (m_fstab_list[x]);
}

void MountTables::loadData()
{
    for(int x = m_list.size() - 1; x >= 0; x--)
	delete ( m_list.takeAt(x) );

    for(int x = m_fstab_list.size() - 1; x >= 0; x--)
	delete ( m_fstab_list.takeAt(x) );
    
    mntent *entry;
    FILE *fp;
    QList<mntent *> entries;

    if( (fp = setmntent(_PATH_MOUNTED, "r")) ){
        while( (entry = copyMntent(getmntent(fp))) )
            entries.append(entry);
        
        endmntent(fp);
        
        QListIterator<mntent *> entry_itr(entries);
        while( entry_itr.hasNext() ) 
            m_list.append( new MountEntry(entry_itr.next(), entries) );
    }
    
    if( (fp = setmntent(_PATH_FSTAB, "r") ) ){
        while( (entry = copyMntent(getmntent(fp))) ){
            entries.append(entry);
            m_fstab_list.append( new MountEntry(entry) );
        }
        endmntent(fp);
    }
    
    for(int x = entries.size() - 1 ; x >= 0; x--)
        delete entries[x];
}

QList<MountEntry *> MountTables::getMtabEntries(const QString deviceName)
{
    QList<MountEntry *> device_mounts;
    
    for(int x = m_list.size() - 1; x >= 0; x--){
	if( deviceName == m_list[x]->getDeviceName() )
	    device_mounts.append( m_list.takeAt(x) );
    }

    return device_mounts;
}

QString MountTables::getFstabMountPoint(LogVol *const lv)
{
    return getFstabMountPoint( lv->getMapperPath(), lv->getFilesystemLabel(), lv->getFilesystemUuid() );
}

QString MountTables::getFstabMountPoint(StoragePartition *const partition)
{
    return getFstabMountPoint( partition->getName(), partition->getFilesystemLabel(), partition->getFilesystemUuid() );
}

QString MountTables::getFstabMountPoint(const QString name, const QString label, const QString uuid)
{
    QString entry_name;
    MountEntry *entry;

    QListIterator<MountEntry *> entry_itr(m_fstab_list);
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

// Adds an entry into the mount table file, usually /etc/mtab.

bool MountTables::addMountEntry(const QString device, const QString mountPoint, const QString type, 
                                const QString options, const int dumpFreq, const int pass)
{
    QByteArray device_array      = device.toLocal8Bit();
    QByteArray mount_point_array = mountPoint.toLocal8Bit();
    QByteArray type_array        = type.toLocal8Bit();
    QByteArray options_array     = options.toLocal8Bit();

    const struct mntent mount_entry = { device_array.data(), 
                                        mount_point_array.data(), 
                                        type_array.data(), 
                                        options_array.data(), 
                                        dumpFreq, 
                                        pass };
    
    FILE *const fp = setmntent(_PATH_MOUNTED, "a");

    if(fp){
	if( addmntent(fp, &mount_entry) ){
            fsync( fileno(fp) );
	    endmntent(fp);
	    return false;
	}
	else{              // success
            fsync( fileno(fp) );
	    endmntent(fp);
	    return true;
	}
    }
    else
	return false;
}

bool MountTables::removeMountEntry(const QString mountPoint)
{
    QList<mntent *> mount_entry_list;
    mntent *mount_entry;
    QByteArray new_path(_PATH_MOUNTED);
    new_path.append(".new");     

    const char *mount_table_old = _PATH_MOUNTED;
    const char *mount_table_new = new_path.data();
    
    FILE *fp_old = setmntent(mount_table_old, "r");

/* Multiple devices can be mounted on a directory but only the
   last one mounted will be unmounted at one time. So only the
   last mtab entry gets deleted upon unmounting  */

    while( (mount_entry = copyMntent(getmntent(fp_old))) )
	mount_entry_list.append(mount_entry);

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
    fsync( fileno(fp_new) );
    endmntent(fp_new);

    KDE_rename(mount_table_new, mount_table_old);
    return true;
}
	    
bool MountTables::renameMountEntries(const QString oldName, const QString newName)
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
	temp_entry = copyMntent(mount_entry);

	if( QString(temp_entry->mnt_fsname) != oldName ){
	    mount_entry_list.append(temp_entry);
	}
	else{
	    temp_entry = buildMntent( newName, 
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
    fsync( fileno(fp_new) );
    endmntent(fp_new);

    KDE_rename(mount_table_new, mount_table_old);

    return true;
}

/* This function generates an mntent structure from its parameters
and returns it  */

mntent* buildMntent(const QString device, const QString mountPoint, const QString type, 
                    const QString options, const int dumpFreq, const int pass)
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

mntent *copyMntent(mntent *const entry)
{
    if( entry == NULL )
        return NULL;

    mntent *new_entry = new mntent;

    new_entry->mnt_fsname = new char[BUFF_LEN]; 
    new_entry->mnt_dir    = new char[BUFF_LEN];
    new_entry->mnt_type   = new char[BUFF_LEN];
    new_entry->mnt_opts   = new char[BUFF_LEN];
    
    strncpy(new_entry->mnt_fsname, entry->mnt_fsname, BUFF_LEN);
    strncpy(new_entry->mnt_dir,    entry->mnt_dir,    BUFF_LEN);
    strncpy(new_entry->mnt_type,   entry->mnt_type,   BUFF_LEN);
    strncpy(new_entry->mnt_opts,   entry->mnt_opts,   BUFF_LEN);
    
    new_entry->mnt_freq   = entry->mnt_freq;
    new_entry->mnt_passno = entry->mnt_passno;
    
    return new_entry;
}

