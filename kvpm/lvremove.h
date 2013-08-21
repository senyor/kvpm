/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef LVREMOVE_H
#define LVREMOVE_H


#include <QStringList>

#include "kvpmdialog.h"

class LogVol;


class LVRemoveDialog : public KvpmDialog
{
    Q_OBJECT

    QString m_name;
    LogVol *m_lv;

    QStringList getDependentChildren(LogVol *const lv);

public:
    explicit LVRemoveDialog(LogVol *const lv, QWidget *parent = nullptr);

private slots:
    void commit();

};

#endif

