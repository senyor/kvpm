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

#ifndef VGREDUCE_H
#define VGREDUCE_H

#include <KDialog>
#include <QCheckBox>
#include <QStringList>


class VolGroup;
class PVCheckBox;

bool reduce_vg(VolGroup *volumeGroup);


class VGReduceDialog : public KDialog
{
Q_OBJECT

    VolGroup *m_vg;
    PVCheckBox *m_pv_checkbox; 
    bool m_unremovable_pvs_present;

 public:
    explicit VGReduceDialog(VolGroup *volumeGroup, QWidget *parent = 0);
    QStringList arguments();
    
 private slots:
    void excludeOneVolume();  // one pv must remain in the vg
    void commitChanges();
};

#endif
