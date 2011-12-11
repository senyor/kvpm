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

#include <QFrame>

class StorageDevice;
class StoragePartition;
class PhysVol;

class DeviceProperties : public QWidget
{
    QFrame *hardwareFrame(StorageDevice *const device);
    QFrame *pvFrame(PhysVol *const pv);
    QFrame *mpFrame(StoragePartition *const partition);
    QFrame *fsFrame(StoragePartition *const partition, const bool showFsUuid, const bool showFsLabel);
    QFrame *generalFrame(StorageDevice *const device);
    QFrame *generalFrame(StoragePartition *const partition);

 public:
     explicit DeviceProperties(StorageDevice *const device, QWidget *parent = 0);
     explicit DeviceProperties(StoragePartition *const partition, QWidget *parent = 0);
};

#endif
