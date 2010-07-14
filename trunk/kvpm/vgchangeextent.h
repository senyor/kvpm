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

#ifndef VGCHANGEEXTENT_H
#define VGCHANGEEXTENT_H

#include <KDialog>

#include <QStringList>
#include <QComboBox>

class VolGroup;

bool change_vg_extent(VolGroup *volumeGroup);

class VGChangeExtentDialog : public KDialog
{
Q_OBJECT
    QString m_vg_name;

    QComboBox *m_extent_size, 
	      *m_extent_suffix;

public:
    VGChangeExtentDialog(VolGroup *volumeGroup, QWidget *parent = 0);

private slots:
    void commitChanges();
    void limitExtentSize(int index);    
};

#endif
