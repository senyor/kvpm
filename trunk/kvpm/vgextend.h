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

#ifndef VGEXTEND_H
#define VGEXTEND_H

#include <QStringList>
#include <KDialog>
#include <QCheckBox>
#include <QVBoxLayout>

#include "misc.h"
#include "volgroup.h"

bool extend_vg(QString volumeGroupName, QString physicalVolumeName, long long size);

bool extend_vg(VolGroup *volumeGroup);

class VGExtendDialog : public KDialog
{
Q_OBJECT

    QVBoxLayout *m_layout;
    QList<NoMungeCheck *> m_pv_checks;
    QStringList m_pv_names;
    VolGroup *m_vg;

 public:
    VGExtendDialog(VolGroup *volumeGroup, QStringList physcialVolumeNames, QWidget *parent = 0);
    
 private slots:
    void commitChanges();
    void validateOK();
    void selectAll();
    void selectNone();
    
};

#endif
