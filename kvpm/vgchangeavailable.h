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

#ifndef VGCHANGEAVAILABLE_H
#define VGCHANGEAVAILABLE_H

#include <KDialog>

#include <QStringList>
#include <QRadioButton>

class VolGroup;

bool change_vg_available(VolGroup *volumeGroup);


class VGChangeAvailableDialog : public KDialog
{
Q_OBJECT
    VolGroup *m_vg;

    QRadioButton *m_available, *m_unavailable;

 public:
    VGChangeAvailableDialog(VolGroup *volumeGroup, QWidget *parent = 0);
    QStringList args();

};

#endif
