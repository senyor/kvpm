/*
 *
 * 
 * Copyright (C) 2008, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "fsprobe.h"

#include <blkid/blkid.h>

#include <QtGui>


#define BLKID_EMPTY_CACHE       "/dev/null"

QString fsprobe_getfstype2(QString devicePath)
{
    static blkid_cache blkid2;
    const QByteArray path = devicePath.toAscii();
    QString fs_type;
    blkid2 = NULL;
    
    if( blkid_get_cache(&blkid2, BLKID_EMPTY_CACHE) < 0){
	qDebug() << "blkid2 cache could not be allocated?";
	return QString();
    }
    else{
	fs_type = QString( blkid_get_tag_value(blkid2, "TYPE", path.data() ) );
	blkid_put_cache(blkid2);
	return fs_type;
    }
}

QString fsprobe_getfsuuid(QString devicePath)
{
    static blkid_cache blkid2;
    const QByteArray path = devicePath.toAscii();
    QString fs_uuid;
    blkid2 = NULL;
    
    if( blkid_get_cache(&blkid2, BLKID_EMPTY_CACHE) < 0){
	qDebug() << "blkid2 cache could not be allocated?";
	return QString();
    }
    else{
	fs_uuid = QString( blkid_get_tag_value(blkid2, "UUID", path.data() ) );
	blkid_put_cache(blkid2);
	return fs_uuid;
    }
}

QString fsprobe_getfslabel(QString devicePath)
{
    static blkid_cache blkid2;
    const QByteArray path = devicePath.toAscii();
    QString fs_label;
    blkid2 = NULL;
    
    if( blkid_get_cache(&blkid2, BLKID_EMPTY_CACHE) < 0){
	qDebug() << "blkid2 cache could not be allocated?";
	return QString();
    }
    else{
        fs_label = QString( blkid_get_tag_value(blkid2, "LABEL", path.data() ) );
	blkid_put_cache(blkid2);
	return fs_label;
    }
}
