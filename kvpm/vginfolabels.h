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
#ifndef VGINFOLABELS_H
#define VGINFOLABELS_H

#include <QWidget>
#include "volgroup.h"

class VGInfoLabels : public QWidget
{
 public:
     VGInfoLabels(VolGroup *VolumeGroup, QWidget *parent = 0);
    
};

#endif
