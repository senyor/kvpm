/*
 *
 * 
 * Copyright (C) 2008, 2010 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef VGCHANGERESIZE_H
#define VGCHANGERESIZE_H

#include <KDialog>

#include <QStringList>
#include <QRadioButton>


class VolGroup;

bool change_vg_resize(VolGroup *VolumeGroup);


class VGChangeResizeDialog : public KDialog
{
Q_OBJECT
    VolGroup *m_vg;

    QRadioButton *m_resize, *m_no_resize;

 public:
    VGChangeResizeDialog(VolGroup *volumeGroup, QWidget *parent = 0);
    QStringList args();

};

#endif
