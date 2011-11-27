/*
 *
 * 
 * Copyright (C) 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include "fsdata.h"

#include <errno.h>
#include <string.h>
#include <sys/statvfs.h>

#include <KLocale>
#include <KMessageBox>

#include <QtGui>

#include "misc.h"


FSData *get_fs_data(QString path){

    struct statvfs *buff = new struct statvfs;

    QByteArray path_array = path.toLocal8Bit();
    const char *mp = path_array.data();
    const int error = statvfs(mp, buff);
 
    if( !error ){
        const long long block_size = buff->f_bsize;
        const long long frag_size  = buff->f_frsize;
        const long long total_blocks = (frag_size * buff->f_blocks) / block_size;

        FSData *fs_data = new FSData();
        fs_data->block_size = block_size; 
        fs_data->size = total_blocks;
        fs_data->used = total_blocks - buff->f_bavail;
        return fs_data;
    }
    else
        return NULL;
}
