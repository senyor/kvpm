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


bool fs_can_reduce(const QString fs);
long long fs_reduce(const QString path, const long long new_size, const QString fs);
long long get_min_fs_size(const QString path, const QString fs);

#endif
