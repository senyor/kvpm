/*
 *
 *
 * Copyright (C) 2008, 2010, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef LVSIZECHARTSEG_H
#define LVSIZECHARTSEG_H

#include <KMenu>

#include <QFrame>

class QHBoxLayout;
class QString;

class VolGroup;
class LogVol;


class LVChartSeg : public QFrame
{
    Q_OBJECT

    VolGroup *m_vg;
    LogVol *m_lv;
    KMenu *m_context_menu;

public:
    LVChartSeg(VolGroup *const group, LogVol *const volume, const QString use, QWidget *parent);

private slots:
    void popupContextMenu(QPoint);

};

#endif
