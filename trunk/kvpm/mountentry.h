/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
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

mntent* buildMountEntry(QString device, QString mountPoint, QString type, 
			QString options, int dumpFreq, int pass);

bool addMountEntryOptions(QString mountPoint, QString newOptions);

bool removeMountEntry(QString mountPoint);
bool hasMountEntry(QString device);
mntent *copyMountEntry(mntent *mountEntry);

QStringList getMountedDevices(QString mountPoint); // Returns devices mounted to mountPoint

QString getFstabEntry(LogVol *const lv);
QString getFstabEntry(StoragePartition *const partition);

bool rename_mount_entries(QString oldName, QString newName);

#endif
