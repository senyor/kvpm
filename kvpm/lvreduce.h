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
#ifndef LVRESIZEDIALOG_H
#define LVRESIZEDIALOG_H

#include <KDialog>
#include <QLineEdit>
#include <QComboBox>
#include "logvol.h"
#include "volgroup.h"

bool LVReduce(LogVol *LogicalVolume);

class LVReduceDialog : public KDialog
{
Q_OBJECT

    LogVol *lv;
    VolGroup *vg;
    QDoubleValidator *size_validator;
    QComboBox *size_combo;
    QLineEdit *size_edit;
    int size_combo_last_index;
    long long new_size, current_size, new_extents;

 public:
    LVReduceDialog(LogVol *LogicalVolume, QWidget *parent = 0);
    QStringList argumentsFS();
    QStringList argumentsLV();
    
     
 private slots:
    void sizeComboAdjust(int index);
    void validateInput(QString text);
 
};

#endif
