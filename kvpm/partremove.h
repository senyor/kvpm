/*
 *
 * 
 * Copyright (C) 2009, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#ifndef PARTREMOVE_H
#define PARTREMOVE_H

#include <QStringList>

class StoragePartition;

bool remove_partition(StoragePartition *const partition);

#endif
