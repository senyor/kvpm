/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "lvsizechart.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QListIterator>
#include <QResizeEvent>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "logvol.h"
#include "lvsizechartseg.h"
#include "volgroup.h"



LVSizeChart::LVSizeChart(VolGroup *const group, QTreeWidget *const vgTree, QWidget *parent) :
    QFrame(parent),
    m_vg(group),
    m_vg_tree(vgTree)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    m_layout->setSizeConstraint(QLayout::SetNoConstraint);

    populateChart();
    setLayout(m_layout);
    setMinimumHeight(45);
    setMaximumHeight(75);

    connect(m_vg_tree->header(), SIGNAL(sectionClicked(int)),
            this, SLOT(vgtreeClicked()));
}

void LVSizeChart::populateChart()
{
    QLayoutItem *child;
    while ((child = m_layout->takeAt(0)))     // remove old children of layout
        delete child;

    LvList volumes;
    const long item_count = m_vg_tree->topLevelItemCount();

    for (int x = 0; x < item_count; ++x) {
        QTreeWidgetItem *const item = m_vg_tree->topLevelItem(x);
        LogVol *const lv = m_vg->getLvByName(item->data(0, Qt::UserRole).toString());

        if (lv) {
            volumes.append(lv);
            if (lv->isThinPool()){
                for (auto thin : lv->getThinVolumes()) {
                    for (auto snap : thin->getSnapshots()) {
                        if (snap->isCowSnap())
                            volumes.append(snap);
                    }
                }
            } else if (lv->getSnapshotCount()) {
                volumes.append(lv->getSnapshots());
            }
        }
    }
    
    const long long free_extents  = m_vg->getFreeExtents();
    const long long total_extents = m_vg->getExtents();
    const long long extent_size   = m_vg->getExtentSize();

    QString usage;                   // Filesystem: blank, ext2 etc. or freespace in vg
    int max_segment_width;
    double seg_ratio;
    QWidget *widget;

    for (auto lv : volumes) {
        if (!lv->isLvmMirrorLeg() && !lv->isThinVolume() &&
            !lv->isLvmMirrorLog() && !lv->isThinPoolData() &&
            !lv->isVirtual()      && !lv->isRaidImage() &&
            !(lv->isLvmMirror()   && !(lv->getOrigin()).isEmpty())) {
 
            if (lv->isThinPool())
                usage = "thin_pool";
            else
                usage = lv->getFilesystem();

            if (lv->isLvmMirror() || lv->isRaid() || lv->isThinPool())
                seg_ratio = (lv->getTotalSize() / (double) extent_size) / (double) total_extents;
            else 
                seg_ratio = lv->getExtents() / (double) total_extents;
            
            m_ratios.append(seg_ratio);
            widget = frameAndConnect(new LVChartSeg(lv, usage, this));
            m_layout->addWidget(widget);
            m_widgets.append(widget);
        }
    }

    if (free_extents) { // only create a free space widget if we have some
        seg_ratio = (free_extents / (double) total_extents) + 0.02; // allow a little "stretch" 0.02
        usage = "freespace" ;
        widget = frameAndConnect(new LVChartSeg(nullptr, usage, this));
        m_widgets.append(widget);

        m_layout->addWidget(widget);
        m_ratios.append(seg_ratio);
    } else if (m_widgets.size() == 0) { // if we have no chart segs then put in a blank one
                                        // because lvsizechart won't work with zero segments
        usage = "" ;
        widget = frameAndConnect(new LVChartSeg(nullptr, usage, this));
        m_widgets.append(widget);

        m_layout->addWidget(widget);
        m_ratios.append(1.0);
    }

    max_segment_width = (int)(width() * m_ratios[0]);

    if (max_segment_width < 1)
        max_segment_width = 1;

    m_widgets[0]->setMaximumWidth(max_segment_width);

    for (int x = m_widgets.size() - 1; x >= 1 ; --x) {
        max_segment_width = qRound(((double)width() * m_ratios[x]));

        if (max_segment_width < 1)
            max_segment_width = 1;

        m_widgets[x]->setMaximumWidth(max_segment_width);
    }
}

void LVSizeChart::resizeEvent(QResizeEvent *event)
{
    const int new_width = (event->size()).width();
    int max_segment_width = (int)(new_width * m_ratios[0]);

    if (max_segment_width < 1)
        max_segment_width = 1;

    m_widgets[0]->setMaximumWidth(max_segment_width);

    for (int x =  m_widgets.size() - 1; x >= 1 ; --x) {
        max_segment_width = qRound(((double)width() * m_ratios[x]));

        if (max_segment_width < 1)
            max_segment_width = 1;

        m_widgets[x]->setMaximumWidth(max_segment_width);
    }
}

void LVSizeChart::vgtreeClicked()
{
    populateChart();
}

QFrame* LVSizeChart::frameAndConnect(LVChartSeg *const seg) 
{
    connect(seg,  SIGNAL(lvMenuRequested(LogVol *)),
            this, SIGNAL(lvMenuRequested(LogVol *)));

    QFrame *const frame = new QFrame();
    frame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    frame->setLineWidth(2);

    QVBoxLayout *const layout = new QVBoxLayout();
    layout->addWidget(seg);
    layout->setSpacing(0);
    layout->setMargin(0);
    frame->setLayout(layout);

    return frame;
}
