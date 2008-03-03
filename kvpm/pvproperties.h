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

#ifndef PVPROPERTIES_H
#define PVPROPERTIES_H

#include <QWidget>

class PhysVol;

class PVProperties : public QWidget
{
 public:
    PVProperties(PhysVol *physicalVolume, QWidget *parent = 0);

};

#endif
