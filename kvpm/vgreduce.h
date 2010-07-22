/*
 *
 * 
 * Copyright (C) 2008 2010 Benjamin Scott   <benscott@nwlink.com>
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
class NoMungeCheck;

bool reduce_vg(VolGroup *volumeGroup);


class VGReduceDialog : public KDialog
{
Q_OBJECT

    VolGroup *m_vg;
    QList<NoMungeCheck *> m_pv_check_boxes;
    bool m_unremovable_pvs_present;
    
 public:
    VGReduceDialog(VolGroup *volumeGroup, QWidget *parent = 0);
    QStringList arguments();
    
 private slots:
    void excludeOneVolume(bool);  // one pv must remain in the vg
    void commitChanges();
};

#endif
