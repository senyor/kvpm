/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef REMOVEMISSING_H
#define REMOVEMISSING_H

#include <KDialog>
#include <QRadioButton>
#include <QStringList>

class VolGroup;

bool remove_missing_pv(VolGroup *volumeGroup);

class RemoveMissingDialog : public KDialog
{
Q_OBJECT

    VolGroup *m_vg;

    QRadioButton *m_empty_button, 
                 *m_all_button;
    
 public:
    explicit RemoveMissingDialog(VolGroup *volumeGroup, QWidget *parent = 0);
    QStringList arguments();
    
};

#endif
