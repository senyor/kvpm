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


#ifndef PVMOVE_H
#define PVMOVE_H

#include <KDialog>
#include <QString>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QLabel>

#include "misc.h"

class PhysVol;
class LogVol;
class VolGroup;


bool move_pv(PhysVol *physicalVolume);
bool move_pv(LogVol *logicalVolume);
bool restart_pvmove();
bool stop_pvmove();

class PVMoveDialog : public KDialog
{
Q_OBJECT

    QLabel *free_space_total_label;
    
    LogVol   *m_lv;
    long long m_pv_used_space;

    QList<PhysVol *> m_source_pvs;    // source physical volumes
    QList<PhysVol *> m_target_pvs;    // destination physical volumes
    QList<NoMungeCheck *> m_check_boxes;         // user can select multiple destination pv's
    QList<NoMungeRadioButton *> m_radio_buttons; // user can select one source pv
    bool move_lv;

    void buildDialog();
    void removeEmptyTargets();
    
public:
    PVMoveDialog(PhysVol *physicalVolume, QWidget *parent = 0);
    PVMoveDialog(LogVol *logicalVolume, QWidget *parent = 0);
    QStringList arguments();
    
private slots:
    void calculateSpace(bool checked);
    void disableDestination(bool checked);
    void checkBoxEnable(bool checked);
    void selectAll();
    void selectNone();

};

#endif
