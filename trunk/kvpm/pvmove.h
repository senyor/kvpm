/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
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

#include "nomungecheck.h"

class PhysVol;
class LogVol;
class VolGroup;


bool move_pv(PhysVol *PhysicalVolume);
bool move_pv(LogVol *LogicalVolume);

class PVMoveDialog : public KDialog
{
Q_OBJECT

    QLabel *free_space_total_label;
    QLabel *needed_space_total_label;
    
    long long needed_space_total, free_space_total;
    QList<long long> free_space;
    LogVol *lv;
    VolGroup *vg;
    QStringList source_pvs;                // full path to source physical volumes
    QStringList destination_pvs;           // same for destination physical volumes
    QList<NoMungeCheck *> check_boxes;     // user can select multiple destination pv's
    QList<QRadioButton *> radio_buttons;   // user can select one source pv
    QCheckBox *check_box_any;              // use any destination pv
    bool move_lv;

    void buildDialog();
    
public:
    PVMoveDialog(PhysVol *PhysicalVolume, QWidget *parent = 0);
    PVMoveDialog(LogVol *LogicalVolume, QWidget *parent = 0);
    QStringList arguments();
    
private slots:
    void calculateSpace(bool checked);
    void disableDestination(bool checked);
    void checkBoxEnable(bool checked);
};

#endif
