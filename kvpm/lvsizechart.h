/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Klvm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */
#ifndef LVSIZECHART_H
#define LVSIZECHART_H

#include <KMenu>
#include <QHBoxLayout>
#include <QFrame>

class VolGroup;
class LogVol;


class LVSizeChart : public QFrame
{
Q_OBJECT
    VolGroup *vg;
    LogVol *lv;
    QHBoxLayout *layout;
    QList<QWidget *> widgets;    // These are segments of the bar chart
    QList<double> ratios;        // These are the relative size of each segment
                                 // to the whole chart. The total should be about 1.

 public:
    LVSizeChart(VolGroup *VolumeGroup, QWidget *parent = 0);
    void populateChart();
    void resizeEvent(QResizeEvent *event);
};

#endif
