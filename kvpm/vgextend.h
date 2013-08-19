/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
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

#include <QList>

#include "kvpmdialog.h"

class PvGroupBox;
class StorageBase;
class VolGroup;


class VGExtendDialog : public KvpmDialog
{
    Q_OBJECT

    bool m_bailout;
    PvGroupBox *m_pv_checkbox = nullptr;
    VolGroup   *m_vg = nullptr;

    bool continueWarning();
    void buildDialog(QList<StorageBase *> devices);
    QList<StorageBase *> getUsablePvs();

public:
    VGExtendDialog(VolGroup *const group, QWidget *parent = nullptr);
    VGExtendDialog(VolGroup *const group, StorageBase *const device, QWidget *parent = nullptr);

private slots:
    void commit();
    void validateOK();

};

#endif
