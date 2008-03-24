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

#ifndef LVPROPERTIES_H
#define LVPROPERTIES_H

#include <QWidget>

class LogVol;

class LVProperties : public QWidget
{
 public:
    LVProperties(LogVol *logicalVolume, int segment, QWidget *parent = 0);

};

#endif
