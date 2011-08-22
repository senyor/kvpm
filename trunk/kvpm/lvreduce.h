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

#ifndef LVREDUCEDIALOG_H
#define LVREDUCEDIALOG_H

#include <KDialog>
#include <KLineEdit>
#include <KComboBox>
#include <KDoubleValidator>

class LogVol;
class VolGroup;

bool lv_reduce(LogVol *logicalVolume);


class LVReduceDialog : public KDialog
{
Q_OBJECT

    LogVol   *m_lv;
    VolGroup *m_vg;

    KDoubleValidator *m_size_validator;
    KComboBox *m_size_combo;
    KLineEdit *m_size_edit;
 
    int m_size_combo_last_index;
    long long m_current_lv_size;
    long long m_min_lv_size;

    long long getSizeEditExtents(int index);

 public:
    explicit LVReduceDialog(LogVol *logicalVolume, QWidget *parent = 0);
     
 private slots:
    void sizeComboAdjust(int index);
    void validateInput(QString text);
    void doShrink();
 
};

#endif
