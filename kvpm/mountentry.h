/*
 *
 * 
 * Copyright (C) 2008, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef MOUNTENTRY_H
#define MOUNTENTRY_H

#include <QString>

class mntent;
class LogVol;
class StoragePartition;

bool addMountEntry(QString device, QString mountPoint, QString type, 
		  QString options, int dumpFreq, int pass);

mntent *copyMountEntry(mntent *mountEntry);

bool removeMountEntry(QString mountPoint);
bool rename_mount_entries(QString oldName, QString newName);

#endif
