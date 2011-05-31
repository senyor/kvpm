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

#ifndef REMOVEFS_H
#define REMOVEFS_H


class StoragePartition;
class LogVol;

bool remove_fs(StoragePartition *partition);
bool remove_fs(LogVol *logicalVolume);

#endif
