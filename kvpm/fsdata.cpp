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

#include <errno.h>
#include <string.h>
#include <sys/statvfs.h>

#include <KLocale>
#include <KMessageBox>

#include <QtGui>

#include "fsdata.h"
#include "misc.h"

FSData *get_fs_data(QString path){

    long long total_blocks, frag_size, block_size;
    int error;

    FSData *fs_data = new FSData();
    fs_data->size = -1;
    fs_data->used = -1;
    fs_data->block_size = -1;

    struct statvfs *buf;
    buf = new struct statvfs;

    QByteArray path_array = path.toAscii();
    const char *mp = path_array.data();
    error =  statvfs(mp, buf);
 
    if(!error){  // We use "long long" intermediate variables here
        block_size = buf->f_bsize;
        frag_size = buf->f_frsize;
        total_blocks = (frag_size * buf->f_blocks) / block_size;

        fs_data->block_size = block_size; 
        fs_data->size = total_blocks;
        fs_data->used = total_blocks - buf->f_bavail;
    }

    return fs_data;
}
