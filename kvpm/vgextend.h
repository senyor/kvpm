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

#ifndef VGEXTEND_H
#define VGEXTEND_H

#include <QStringList>
#include <KDialog>
#include <QCheckBox>
#include <QVBoxLayout>

class PVCheckBox;
class StorageDevice;
class StoragePartition;
class VolGroup;

bool extend_vg(QString volumeGroupName, StorageDevice *device, StoragePartition *partition);

bool extend_vg(VolGroup *volumeGroup);

class VGExtendDialog : public KDialog
{
Q_OBJECT

    PVCheckBox  *m_pv_checkbox;
    VolGroup    *m_vg;
    QVBoxLayout *m_layout;

 public:
    VGExtendDialog(VolGroup *volumeGroup, QList<StorageDevice *> devices, 
                   QList<StoragePartition *> partitions, QWidget *parent = 0);
    
 private slots:
    void commitChanges();
    void validateOK();
    
};

#endif
