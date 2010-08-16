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

#ifndef VGCHANGEALLOC_H
#define VGCHANGEALLOC_H

#include <KDialog>
#include <QStringList>
#include <QRadioButton>

class VGChangeAllocDialog : public KDialog
{
Q_OBJECT

    QString m_vg_name;
    QRadioButton *m_normal, *m_contiguous, *m_anywhere, *m_cling;;

public:
    VGChangeAllocDialog(QString volumeGroupName, QWidget *parent = 0);
    QStringList arguments();
    
};

#endif