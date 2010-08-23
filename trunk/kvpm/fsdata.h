/*
 *
 * 
 * Copyright (C) 2010 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef FSDATA_H
#define FSDATA_H

#include <QString>

struct FSData {
    long long size;
    long long used;
    long long block_size;
};

FSData *get_fs_data(QString path);

#endif
