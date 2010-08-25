/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <KSharedConfig>
#include <KConfigGroup>

#include <QtGui>

#include "executablefinder.h"

/* The purpoise of this class is to map the name of a program
   with the full path of the executable */

ExecutableFinder::ExecutableFinder(QObject *parent) : QObject(parent)
{
    m_keys << "dumpe2fs"
           << "fsck"
           << "lvchange"
	   << "lvconvert" 
	   << "lvcreate"
	   << "lvextend"
	   << "lvreduce"
	   << "lvremove"
	   << "lvrename"
	   << "lvs"
	   << "mkfs"
	   << "mkswap"
	   << "pvchange"
	   << "pvcreate"
	   << "pvmove" 
	   << "pvremove"
	   << "pvresize"
	   << "pvs"
	   << "resize2fs" 
	   << "resize_reiserfs"
           << "udevadm"
	   << "vgchange"
	   << "vgcreate"
	   << "vgexport"
	   << "vgextend"
	   << "vgimport"
	   << "vgmerge"
	   << "vgreduce"
	   << "vgremove"
	   << "vgrename"
	   << "vgs" 
           << "vgsplit" 
	   << "xfs_growfs";

    m_default_search_paths << "/sbin/" 
			   << "/usr/sbin/" 
			   << "/bin/" 
			   << "/usr/bin/" 
			   << "/usr/local/bin/"
			   << "/usr/local/sbin/";
    
    reload();

}

QString ExecutableFinder::getExecutablePath(QString name)
{
    QString path = m_path_map.value(name);

    if(path == "")
	qDebug() << "Excutable Finder: error " << name  << " does not map to any path";
    
    return path;
}

void ExecutableFinder::reload()
{
    struct stat buf;
    const char *path;
    int key_length = m_keys.size();

    // Read the kvpmrc files and get the search paths or use the default
    // if none are found, then write out the default. 

    KConfig kvpm_config( "kvpmrc", KConfig::SimpleConfig );
    KConfigGroup system_paths_group( &kvpm_config, "SystemPaths" );

    QStringList search_paths = system_paths_group.readEntry( "SearchPath", QStringList() );
    if( search_paths.isEmpty() ){
        search_paths = m_default_search_paths ;
        system_paths_group.writeEntry( "SearchPath",    search_paths );
        system_paths_group.sync();
    }

    m_path_map.clear();

    for(int y = 0; y < key_length; y++){
	for(int x = 0; x < search_paths.size(); x++){
	    path = QString( search_paths[x] + m_keys[y] ).toAscii().data();
	    if( lstat(path, &buf) == 0 ){
		m_path_map.insert( m_keys[y], search_paths[x] + m_keys[y] );
		break;
	    }
	}
    }

    m_not_found.clear();
    for(int x = 0; x < key_length; x++){
        if( m_path_map.value( m_keys[x] ) == "" )
	    m_not_found.append( m_keys[x] );
    }
}


QStringList ExecutableFinder::getAllPaths()
{
    return m_path_map.values();
}


QStringList ExecutableFinder::getAllNames()
{
    return   m_path_map.keys();
}

QStringList ExecutableFinder::getNotFound()
{
    return m_not_found;
}
