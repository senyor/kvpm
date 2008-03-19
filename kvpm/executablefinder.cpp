/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
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

#include <QtGui>

#include "executablefinder.h"

/* The purpoise of this class is to map he name of a program
   with the full path of the executable */

ExecutableFinder::ExecutableFinder(QObject *parent) : QObject(parent)
{
    struct stat buf;
    QStringList keys;
    QStringList search_path;
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
	 << "vgextend"
	 << "vgreduce"
	 << "vgremove"
	 << "vgs" 
	 << "xfs_growfs";
    
    
    search_path << "/sbin/" 
		<< "/usr/sbin/" 
		<< "/bin/" 
		<< "/usr/bin/" 
		<< "/usr/local/bin/";
    

    for(int y = 0; y < keys.size(); y++){
	for(int x = 0; x < search_path.size(); x++){
	    path = QString( search_path[x] + keys[y] ).toAscii().data();
	    if( lstat(path, &buf) == 0 ){
		m_path_map.insert( keys[y], search_path[x] + keys[y] );
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
