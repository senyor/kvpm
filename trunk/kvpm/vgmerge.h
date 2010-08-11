/*
 *
 * 
 * Copyright (C) 2010 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef VGMERGE_H
#define VGMERGE_H

#include <QCheckBox>

#include <KDialog>
#include <KComboBox>

class VolGroup;

bool merge_vg(VolGroup *volumeGroup);

class VGMergeDialog : public KDialog
{
Q_OBJECT

    VolGroup  *m_vg;
    KComboBox *m_target_combo;
    QCheckBox *m_autobackup;

 public:
    VGMergeDialog(VolGroup *volumeGroup, QWidget *parent = 0);
    QStringList arguments();    

};

#endif
