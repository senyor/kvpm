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

#ifndef LVREMOVE_H
#define LVREMOVE_H

#include <QStringList>

class LogVol;

bool remove_lv(LogVol *logicalVolume);
bool remove_lv(QString fullName);

#endif
