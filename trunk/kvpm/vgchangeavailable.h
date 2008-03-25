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

#ifndef VGCHANGEAVAILABLE_H
#define VGCHANGEAVAILABLE_H

#include <KDialog>
#include <QStringList>
#include <QCheckBox>

class VolGroup;


class VGChangeAvailableDialog : public KDialog
{
Q_OBJECT

    QString    m_vg_name;
    QCheckBox *m_avail_check;

public:
    VGChangeAvailableDialog(VolGroup *volumeGroup, QWidget *parent = 0);
    QStringList arguments();
    
};

#endif
