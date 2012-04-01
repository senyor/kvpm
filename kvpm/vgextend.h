/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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

#include <KDialog>

#include <QStringList>

class PvGroupBox;
class StorageDevice;
class StoragePartition;
class VolGroup;


class VGExtendDialog : public KDialog
{
    Q_OBJECT

    bool m_bailout;
    PvGroupBox  *m_pv_checkbox;
    VolGroup    *m_vg;

public:
    VGExtendDialog(VolGroup *const group, QWidget *parent = NULL);
    VGExtendDialog(VolGroup *const group, StorageDevice *const device, StoragePartition *const partition, QWidget *parent = NULL);
    void buildDialog(QList<StorageDevice *> devices, QList<StoragePartition *> partitions);
    void getUsablePvs(QList<StorageDevice *> &devices, QList<StoragePartition *> &partitions);
    bool bailout();

private slots:
    void commitChanges();
    void validateOK();

};

#endif
