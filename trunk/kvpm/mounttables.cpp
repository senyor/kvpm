/*
 *
 * 
 * Copyright (C) 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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


const int BUFF_LEN = 2000;   // Enough?


mntent *buildMntent(const QString device, const QString mountPoint, const QString type, 
                    const QString options, const int dumpFreq, const int pass);

mntent *copyMntent(mntent *const entry);
void  deleteMntent(mntent *const entry);

bool isMtabWritable(QString mtab)
{
    QFileInfo fi = QFileInfo(mtab);

    if( fi.isSymLink() )
        return false;
    else
        return fi.isWritable();
}

// Static init
bool MountTables::can_write_mtab = isMtabWritable(_PATH_MOUNTED);


MountTables::MountTables()
{
}

MountTables::~MountTables()
{
    for(int x = 0; x < m_fstab_list.size(); x++)
	delete (m_fstab_list[x]);

    for(int x = 0; x < m_mount_list.size(); x++)
	delete (m_mount_list[x]);
}

void MountTables::loadData()
{
    for(int x = m_fstab_list.size() - 1; x >= 0; x--)
	m_fstab_list.takeAt(x)->deleteLater();

    for(int x = m_mount_list.size() - 1; x >= 0; x--)
	m_mount_list.takeAt(x)->deleteLater();

    QString line;
    QFile file("/proc/self/mountinfo");

    if ( file.open(QIODevice::ReadOnly | QIODevice::Text) ){

        QTextStream in( &file );

        while ( ! (line = in.readLine()).isEmpty() ){
            QStringList subs = line.section(' ', 2, 2).split(':');  // major and minor dev numbers
            
            m_mount_list.append( new MountEntry(line, subs[0].toInt(), subs[1].toInt()) );
        }
    }

    file.close();

    for(int x = m_mount_list.size() - 1; x >= 0; x--){
        int pos = 1;

        if( m_mount_list[x]->getMountPosition() > 0 )
            continue;

        for(int y = x - 1; y >= 0; y--){
            if(m_mount_list[y]->getMountPoint() == m_mount_list[x]->getMountPoint() )
                m_mount_list[y]->setMountPosition(++pos);
        }
        if(pos > 1)
            m_mount_list[x]->setMountPosition(1);
    }

    mntent *entry;
    FILE *fp;

    if( (fp = setmntent(_PATH_FSTAB, "r") ) ){

        while( (entry = getmntent(fp)) )
            m_fstab_list.append( new MountEntry(entry) );

        endmntent(fp);
    }
}

// Major and minor numbers don't change when lv or vg names are changed
// Also lookups by fs uuid won't work with snaps since they will match the origin
// and each other. 
QList<MountEntry *> MountTables::getMtabEntries(const int major, const int minor)
{
    QList<MountEntry *> device_mounts;

    for(int x = 0; x < m_mount_list.size(); x++){
        if(m_mount_list[x]->getMajorNumber() == major && m_mount_list[x]->getMinorNumber() == minor){
            device_mounts.append( new MountEntry(m_mount_list[x]) );
        }
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

// Adds an entry (a mntent struct) into the mount table file, usually /etc/mtab.

bool MountTables::addEntry(const QString device, const QString mountPoint, const QString type, 
                           const QString options, const int dumpFreq, const int pass)
{
    if(!can_write_mtab)
        return true;

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
            fchmod(fileno(fp), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            fsync(fileno(fp));
	    endmntent(fp);
	    return false;
	}
	else{              // success
            fchmod(fileno(fp), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            fsync( fileno(fp) );
	    endmntent(fp);
	    return true;
	}
    }
    else
	return false;
}

bool MountTables::removeEntry(const QString mountPoint)
{
    if(!can_write_mtab)
        return true;

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
            deleteMntent(mount_entry_list.takeAt(x));
	    break;
	}
    }
    endmntent(fp_old);

    FILE *fp_new = setmntent(mount_table_new, "w");
    for(int x = 0; x < mount_entry_list.size(); x++){
	const mntent *entry = mount_entry_list[x];
	addmntent(fp_new, entry);
    }
    fchmod(fileno(fp_new), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    fsync( fileno(fp_new) );
    endmntent(fp_new);

    KDE_rename(mount_table_new, mount_table_old);
    return true;
}
	    
bool MountTables::renameEntries(const QString oldName, const QString newName)
{
    if(!can_write_mtab)
        return true;

    QList<mntent *> mount_entry_list;
    mntent *mount_entry;
    mntent *temp_entry;

    QByteArray new_path(_PATH_MOUNTED);
    new_path.append(".new");     

    const char *mount_table_old = _PATH_MOUNTED;
    const char *mount_table_new = new_path.data();
    
    FILE *fp_old = setmntent(mount_table_old, "r");

    while( (mount_entry = copyMntent( getmntent(fp_old)) ) ){

	if( QString(mount_entry->mnt_fsname) != oldName ){
	    mount_entry_list.append(mount_entry);
	}
	else{
	    temp_entry = buildMntent( newName, 
                                      mount_entry->mnt_dir,
                                      mount_entry->mnt_type,	 
                                      mount_entry->mnt_opts,
                                      mount_entry->mnt_freq,
                                      mount_entry->mnt_passno );
	    deleteMntent(mount_entry);
	    mount_entry_list.append(temp_entry);
	}
    }
    
    endmntent(fp_old);

    FILE *fp_new = setmntent(mount_table_new, "w");
    for(int x = 0; x < mount_entry_list.size(); x++){
	const mntent *entry = mount_entry_list[x];
	addmntent(fp_new, entry);
    }
    fchmod(fileno(fp_new), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    fsync(fileno(fp_new));
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

void  deleteMntent(mntent *const entry)
{
    if(entry != NULL){
        delete entry->mnt_fsname;
        delete entry->mnt_dir;
        delete entry->mnt_type;
        delete entry->mnt_opts;
        delete entry;
    }
}
