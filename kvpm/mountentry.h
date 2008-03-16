/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
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

int addMountEntry(QString Device, QString MountPoint, QString Type, 
		  QString Options, int DumpFreq, int Pass);

bool addMountEntryOptions(QString MountPoint, QString NewOptions);

bool removeMountEntry(QString MountPoint);
mntent *copyMountEntry(mntent *MountEntry);

bool hasMountEntry(QString Device);
bool hasFstabEntry(QString Device);

QString getMountEntry(QString Device);
QString getFstabEntry(QString Device);


#endif
