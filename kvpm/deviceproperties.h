/*
 *
 * 
 * Copyright (C) 2008, 2009 Benjamin Scott   <benscott@nwlink.com>
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
     DeviceProperties( StorageDevice *Device, QWidget *parent = 0);
     DeviceProperties( StoragePartition *Partition, QWidget *parent = 0);
};

#endif