/*
 *
 * 
 * Copyright (C) 2009, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef FSREDUCE_H
#define FSREDUCE_H

#include <QString>


long long fs_reduce(QString path, long long new_size, QString fs);
long long get_min_fs_size(QString path, QString fs);

#endif
