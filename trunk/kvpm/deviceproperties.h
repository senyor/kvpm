/*
 *
 * 
 * Copyright (C) 2008, 2009, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#ifndef DEVICEPROPERTIES_H
#define DEVICEPROPERTIES_H

#include <QWidget>

class StorageDevice;
class StoragePartition;


class DeviceProperties : public QWidget
{

 public:
     explicit DeviceProperties(StorageDevice *device, QWidget *parent = 0);
     explicit DeviceProperties(StoragePartition *partition, QWidget *parent = 0);
};

#endif
