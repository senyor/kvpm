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
#ifndef VGCHANGELV_H
#define VGCHANGELV_H

#include <KDialog>
#include <QStringList>
#include <QSpinBox>
#include <QGroupBox>

class VolGroup;

bool change_vg_lv(VolGroup *VolumeGroup);


class VGChangeLVDialog : public KDialog
{
    QString vg_name;
    VolGroup *vg;
    QGroupBox *limit_lvs;
    QSpinBox *max_lvs;
    
public:
    VGChangeLVDialog(VolGroup *VolumeGroup, QWidget *parent = 0);
    QStringList arguments();
};

#endif
