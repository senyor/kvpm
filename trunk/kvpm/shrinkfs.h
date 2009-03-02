/*
 *
 * 
 * Copyright (C) 2009 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef SHRINKFS_H
#define SHRINKFS_H

#include <QString>

#include <parted/parted.h>

#include "storagepartition.h"

PedSector shrink_fs(QString path, PedSector new_size, QString fs);
PedSector get_min_fs_size(QString path, QString fs);


#endif
