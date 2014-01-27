/*
 *
 *
 * Copyright (C) 2008, 2010, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
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

#include "typedefs.h"

class QHBoxLayout;
class QBrush;
class QString;

class LogVol;


class LVChartSeg : public QWidget
{
    Q_OBJECT

    LvPtr m_lv = nullptr;
    QBrush m_brush;

public:
    LVChartSeg(LvPtr volume, const QString use, QWidget *parent);
    void paintEvent(QPaintEvent *);

private slots:
    void popupContextMenu(QPoint);

signals:
    void lvMenuRequested(LogVol *lv);

};

#endif
