/*
 *
 *
 * Copyright (C) 2008, 2011, 2012, 2013, 2014 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef LVSIZECHART_H
#define LVSIZECHART_H

#include <QFrame>

#include "typedefs.h"


class QHBoxLayout;
class QResizeEvent;
class QTreeWidget;

class VolGroup;
class LogVol;
class LVChartSeg;

class LVSizeChart : public QFrame
{
    Q_OBJECT

    VolGroup *m_vg = nullptr;
    QTreeWidget  *m_vg_tree = nullptr;
    QHBoxLayout  *m_layout = nullptr;

    QList<QWidget *> m_widgets;    // These are segments of the bar chart
    QList<double>    m_ratios;     // These are the relative size of each segment
                                   // to the whole chart. The total should be about 1.

    QFrame *frameAndConnect(LVChartSeg *const seg); 

public:
    LVSizeChart(VolGroup *const group, QTreeWidget *const vgTree, QWidget *parent = nullptr);
    void populateChart();
    void resizeEvent(QResizeEvent *event);

private slots:
    void vgtreeClicked();

signals:
    void lvMenuRequested(LvPtr lv);

};

#endif
