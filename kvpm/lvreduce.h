/*
 *
 * 
 * Copyright (C) 2008 ,2011 Benjamin Scott   <benscott@nwlink.com>
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
class SizeSelectorBox;
class VolGroup;

bool lv_reduce(LogVol *logicalVolume);


class LVReduceDialog : public KDialog
{
Q_OBJECT

    LogVol   *m_lv;
    VolGroup *m_vg;
    SizeSelectorBox *m_size_selector;
    bool m_bailout;

 public:
    explicit LVReduceDialog(LogVol *const logicalVolume, QWidget *parent = 0);
    bool bailout();
     
 private slots:
    void doShrink();
 
};

#endif
