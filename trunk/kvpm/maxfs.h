/*
 *
 *
 * Copyright (C) 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef MAXFS_H
#define MAXFS_H

class LogVol;
class StoragePartition;
class StorageDevice;

bool max_fs(LogVol *const logicalVolume);
bool max_fs(StoragePartition *const partition);
bool max_fs(StorageDevice *const device);

#endif
