/*
 *
 *
 * Copyright (C) 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef REPAIRMISSING_H
#define REPAIRMISSING_H

#include <KDialog>

#include <QStringList>

class QCheckBox;
class QGroupBox;
class QWidget;

#include "logvol.h"

class PhysVol;
class PvGroupBox;


class RepairMissingDialog : public KDialog
{
    Q_OBJECT

    PvGroupBox *m_pv_box;
    QCheckBox  *m_replace_box;

    bool m_bailout;
    LogVol *m_lv;         // The volume we are repairing

    QStringList arguments();
    QList<PhysVol *> getUsablePvs();
    QList<PhysVol *> getSelectedPvs();
    LogVolList  getPartialLvs();
    QWidget *buildPhysicalWidget(QList<PhysVol *> const pvs);
    int getImageNumber(QString name);

public:
    explicit RepairMissingDialog(LogVol *const volume, QWidget *parent = NULL);
    bool bailout();  // if true, don't bother executing this dialog

private slots:
    void resetOkButton();
    void commitChanges();
    void setReplace(const bool replace);

};

#endif


