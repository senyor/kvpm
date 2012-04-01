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

#ifndef FSCK_H
#define FSCK_H

class LogVol;
class StoragePartition;

#include <QString>

bool fsck(const QString path);
bool manual_fsck(LogVol *const logicalVolume);
bool manual_fsck(StoragePartition *const partition);

#endif
