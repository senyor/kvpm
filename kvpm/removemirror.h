/*
 *
 *
 * Copyright (C) 2008, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef LVREMOVEMIRROR_H
#define LVREMOVEMIRROR_H


#include <QStringList>

#include "kvpmdialog.h"

class QLabel;

class LogVol;
class NoMungeCheck;
class VolGroup;



class RemoveMirrorDialog : public KvpmDialog
{
    Q_OBJECT

    LogVol *m_lv = nullptr;
    VolGroup *m_vg = nullptr;
    QList<NoMungeCheck *> m_leg_checks;

public:
    explicit RemoveMirrorDialog(LogVol *mirror, QWidget *parent = nullptr);

private slots:
    void validateCheckStates();
    void commit();

};

#endif
