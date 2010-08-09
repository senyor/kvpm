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

#ifndef VGSPLIT_H
#define VGSPLIT_H

#include <KDialog>
#include <KLineEdit>

#include <QStringList>
#include <QCheckBox>
#include <QRegExpValidator>
#include <QTableWidget>

#include "nomungecheck.h"

class VolGroup;
class LogVol;

bool split_vg(VolGroup *volumeGroup);

class VGSplitDialog : public KDialog
{
Q_OBJECT

    QList<NoMungeCheck *> m_pv_checks;
    QList<LogVol *> m_lvs;
    QStringList m_busy_pvs;    // all pvs under busy logical volumes
    QTableWidget *m_pv_table;
    KLineEdit *m_new_vg_name;
    QRegExpValidator *m_validator;
    VolGroup *m_vg;
    
 public:
    VGSplitDialog(VolGroup *volumeGroup, QWidget *parent = 0);
    QStringList arguments();    

 private slots:
    void validateOK();
    void validateName(QString);
    void adjustTable(bool);

};

#endif
