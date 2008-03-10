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

#ifndef PVCHANGE_H
#define PVCHANGE_H

#include <KDialog>
#include <QCheckBox>
#include <QLabel>
#include <QStringList>

class PVChangeDialog : public KDialog
{
Q_OBJECT

     QCheckBox *allocation_box;
     QString m_pv_path;

public:
     PVChangeDialog(QString physicalVolumePath, QWidget *parent = 0);
     QStringList arguments();
 
};

#endif
