/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013, 2014 Benjamin Scott   <benscott@nwlink.com>
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


#include "kvpmdialog.h"

#include <QStringList>
#include <QSharedPointer>

class QStackedWidget;

class PhysVol;
class VolGroup;
class PvGroupBox;
class PvSpace;


class VGReduceDialog : public KvpmDialog
{
    Q_OBJECT

    PhysVol  *m_pv = nullptr;
    const VolGroup *m_vg = nullptr;
    PvGroupBox *m_pv_checkbox = nullptr;
    QStackedWidget *m_error_stack = nullptr;

    QList<QSharedPointer<PvSpace> > getPvSpaceList(); 
    bool hasUnremovablePv();
    bool hasMda(const QStringList remove);
    QStackedWidget *createErrorWidget();

public:
    explicit VGReduceDialog(VolGroup *const group, QWidget *parent = nullptr);
    explicit VGReduceDialog(PhysVol *const pv, QWidget *parent = nullptr);

private slots:
    void resetOkButton();  // one pv must remain in the vg
    void commit();

};

#endif
