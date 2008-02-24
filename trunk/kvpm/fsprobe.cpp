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


#include <blkid/blkid.h>
#include <QtGui>
#include "fsprobe.h"

#define BLKID_EMPTY_CACHE       "/dev/null"

QString fsprobe_getfstype2(QString devicePath)
{
    static blkid_cache blkid2;
    QString fs_type;

    blkid2 = NULL;
    
    if( blkid_get_cache(&blkid2, BLKID_EMPTY_CACHE) < 0){
	qDebug() << "blkid2 cache could not be allocated?";
	return QString();
    }
    else{
	fs_type = QString( blkid_get_tag_value(blkid2, "TYPE", devicePath.toAscii().data() ) );
	blkid_put_cache(blkid2);
	return fs_type;
    }
}
