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

#include <QFrame>

class VolGroup;


class VGInfoLabels : public QFrame
{
 public:
     VGInfoLabels(VolGroup *volumeGroup, QWidget *parent = 0);
    
};

#endif