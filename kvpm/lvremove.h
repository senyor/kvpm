/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013, 2014 Benjamin Scott   <benscott@nwlink.com>
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
#include "typedefs.h"

class LogVol;


class LVRemoveDialog : public KvpmDialog
{
    Q_OBJECT

    QString m_name;
    const LvPtr m_lv;

    QStringList getDependentChildren(const LvPtr lv);

public:
    explicit LVRemoveDialog(const LvPtr lv, QWidget *parent = nullptr);

private slots:
    void commit();

};

#endif

