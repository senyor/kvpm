/*
 *
 *
 * Copyright (C) 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
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

class QGroupBox;
class QRadioButton;
class QWidget;

#include "logvol.h"

class PhysVol;
class PvGroupBox;


class RepairMissingDialog : public KDialog
{
    Q_OBJECT

    PvGroupBox *m_pv_box = nullptr;
    QRadioButton  *m_replace_radio = nullptr;

    bool m_bailout = false;
    LogVol *m_lv = nullptr;         // The volume we are repairing

    QStringList arguments();
    QList<PhysVol *> getUsablePvs();
    QList<PhysVol *> getSelectedPvs();
    LogVolList  getPartialLvs();
    QWidget *buildPhysicalWidget(QList<PhysVol *> const pvs);
    int getImageNumber(QString name);

public:
    explicit RepairMissingDialog(LogVol *const volume, QWidget *parent = nullptr);
    bool bailout();  // if true, don't bother executing this dialog

private slots:
    void resetOkButton();
    void commitChanges();
    void setReplace(const bool replace);

};

#endif


