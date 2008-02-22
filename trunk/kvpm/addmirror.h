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
#ifndef ADDMIRROR_H
#define ADDMIRROR_H

#include <KDialog>
#include <QStringList>
#include <QLabel>
#include <QSpinBox>
#include <QRadioButton>
#include <QGroupBox>


class LogVol;
class VolGroup;

bool add_mirror(LogVol *LogicalVolume);


class AddMirrorDialog : public KDialog
{

     LogVol *lv;
     VolGroup *vg;
     QString logical_volume_name;
     int mirrors;
     QRadioButton *contiguous_button, *normal_button,   //Radio button to chose 
	          *anywhere_button, *inherited_button,  // the allocation policy
	          *cling_button;
     QRadioButton *core_log, *disk_log;
     QSpinBox *count_spin;

 public:
     AddMirrorDialog(LogVol *LogicalVolume, QWidget *parent = 0);
     QStringList arguments();

 };

#endif


