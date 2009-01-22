/*
 *
 * 
 * Copyright (C) 2008, 2009 Benjamin Scott   <benscott@nwlink.com>
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
    struct stat buf;
    QStringList keys;
    QStringList default_search_paths;
    const char *path;

    keys << "lvchange"
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
	 << "pvs"
	 << "resize2fs" 
	 << "resize_reiserfs"
	 << "vgchange"
	 << "vgcreate"
	 << "vgexport"
	 << "vgextend"
	 << "vgimport"
	 << "vgreduce"
	 << "vgremove"
	 << "vgrename"
	 << "vgs" 
	 << "xfs_growfs";
    
    
    default_search_paths << "/sbin/" 
			 << "/usr/sbin/" 
			 << "/bin/" 
			 << "/usr/bin/" 
			 << "/usr/local/bin/"
			 << "/usr/local/sbin/";
    

    // Read the kvpmrc files and get the search paths or use the default
    // if none are found, then write out the default. 

    KConfig kvpm_config( "kvpmrc", KConfig::SimpleConfig );
    KConfigGroup system_paths_group( &kvpm_config, "SystemPaths" );

    QStringList search_paths = system_paths_group.readEntry( "SearchPath", QStringList() );
    if( search_paths.isEmpty() ){
        search_paths = default_search_paths ;
        system_paths_group.writeEntry( "SearchPath",    search_paths );
        system_paths_group.sync();
    }

    for(int y = 0; y < keys.size(); y++){
	for(int x = 0; x < search_paths.size(); x++){
	    path = QString( search_paths[x] + keys[y] ).toAscii().data();
	    if( lstat(path, &buf) == 0 ){
		m_path_map.insert( keys[y], search_paths[x] + keys[y] );
		break;
	    }
	}
    }

}

QString ExecutableFinder::getExecutablePath(QString name)
{
    QString path = m_path_map.value(name);

    if(path == "")
	qDebug() << "Excutable Finder: error " << name << " does not map to any path";
    
    return path;
}
