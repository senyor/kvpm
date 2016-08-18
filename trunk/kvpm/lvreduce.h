/*
 *
 *
 * Copyright (C) 2008, 2011, 2012, 2013, 2016 Benjamin Scott   <benscott@nwlink.com>
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


#include "kvpmdialog.h"


class LogVol;
class SizeSelectorBox;


class LVReduceDialog : public KvpmDialog
{
    Q_OBJECT

    LogVol   *m_lv = nullptr;
    SizeSelectorBox *m_size_selector = nullptr;

public:
    explicit LVReduceDialog(LogVol *const volume, QWidget *parent = nullptr);

private slots:
    void commit();
    void resetOkButton();

};

#endif
