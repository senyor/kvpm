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

#include <QByteArray>
#include <QDebug>
#include <QString>

#include "misc.h"


// Gets basic stats on mounted filesystems like amount used up.

FSData get_fs_data(const QString path)
{

    struct statvfs buff;
    const QByteArray path_qba = path.toLocal8Bit();
    const int error = statvfs(path_qba.data(), &buff);

    FSData fs_data;

    if (error) {
        fs_data.block_size = -1;
        fs_data.size = -1;
        fs_data.used = -1;
    } else {
        const long long block_size = buff.f_bsize;
        const long long frag_size  = buff.f_frsize;
        const long long total_blocks = (frag_size * buff.f_blocks) / block_size;

        fs_data.block_size = block_size;
        fs_data.size = total_blocks;
        fs_data.used = total_blocks - buff.f_bavail;
    }

    return fs_data;
}

