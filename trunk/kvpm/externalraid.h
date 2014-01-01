/*
 *
 *
 * Copyright (C) 2013, 2014 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef DMRAID_H
#define DMRAID_H

#include <QStringList>


/*
  The purpose here is to find the names of raid devices created by
  device mapper or the multiple devices RAID driver and the names
  of the underlying block devices. 
*/


void dmraid_get_devices(QStringList &block, QStringList &raid);
void mdraid_get_devices(QStringList &block, QStringList &raid);

#endif
