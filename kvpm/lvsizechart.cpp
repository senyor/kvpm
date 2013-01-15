/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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
            this , SLOT(vgtreeClicked()));
}

void LVSizeChart::populateChart()
{
    LogVolList logical_volumes;
    QTreeWidgetItem *item;
    LogVol *lv;
    QLayoutItem *child;
    const long item_count = m_vg_tree->topLevelItemCount();

    while ((child = m_layout->takeAt(0)) != 0)     // remove old children of layout
        delete child;

    for (int x = 0; x < item_count; x++) {
        item = m_vg_tree->topLevelItem(x);
        lv = m_vg->getLvByName(item->data(0, Qt::UserRole).toString());

        if (lv != NULL) {
            logical_volumes.append(lv);

            if (lv->isThinPool()){
                QListIterator<QPointer<LogVol> > pool_itr(lv->getThinVolumes());
                while (pool_itr.hasNext()) {
                    LogVolList cow_snaps(pool_itr.next()->getSnapshots());

                    for (int n = cow_snaps.size() - 1; n >= 0; n--) {
                        if (cow_snaps[n]->isCowSnap()) {
                            logical_volumes.append(cow_snaps[n]);
                        }
                    }
                }
            } else if (lv->getSnapshotCount()) {
                logical_volumes.append(lv->getSnapshots());
            }
        }
    }

    double seg_ratio;
    QWidget *widget;

    const long long free_extents  = m_vg->getFreeExtents();
    const long long total_extents = m_vg->getExtents();
    const long long extent_size   = m_vg->getExtentSize();
    const int lv_count = logical_volumes.size();

    QString usage;                   // Filesystem: blank, ext2 etc. or freespace in vg
    int max_segment_width;

    for (int x = 0; x < lv_count; x++) {
        m_lv = logical_volumes[x];

        if (!m_lv->isLvmMirrorLeg() && !m_lv->isThinVolume() &&
            !m_lv->isLvmMirrorLog() && !m_lv->isThinPoolData() &&
            !m_lv->isVirtual()    &&
            !m_lv->isRaidImage() &&
            !(m_lv->isLvmMirror() && !(m_lv->getOrigin()).isEmpty())) {
 
            if (m_lv->isThinPool())
                usage = "thin_pool";
            else
                usage = m_lv->getFilesystem();

            if (m_lv->isLvmMirror() || m_lv->isRaid() || m_lv->isThinPool()){
                seg_ratio = (m_lv->getTotalSize() / (double) extent_size) / (double) total_extents;
            } else {
                seg_ratio = m_lv->getExtents() / (double) total_extents;
            }

            m_ratios.append(seg_ratio);
            widget = buildFrame(new LVChartSeg(m_vg, m_lv, usage, this));
            m_layout->addWidget(widget);
            m_widgets.append(widget);
        }
    }

    if (free_extents) { // only create a free space widget if we have some
        seg_ratio = (free_extents / (double) total_extents) + 0.02; // allow a little "stretch" 0.02
        usage = "freespace" ;
        widget = buildFrame(new LVChartSeg(m_vg, 0, usage, this));
        m_widgets.append(widget);

        m_layout->addWidget(widget);
        m_ratios.append(seg_ratio);
    } else if (m_widgets.size() == 0) { // if we have no chart segs then put in a blank one
                                        // because lvsizechart won't work with zero segments
        usage = "" ;
        widget = buildFrame(new LVChartSeg(m_vg, 0, usage, this));
        m_widgets.append(widget);

        m_layout->addWidget(widget);
        m_ratios.append(1.0);
    }

    max_segment_width = (int)(width() * m_ratios[0]);

    if (max_segment_width < 1)
        max_segment_width = 1;

    m_widgets[0]->setMaximumWidth(max_segment_width);

    for (int x =  m_widgets.size() - 1; x >= 1 ; x--) {
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

    for (int x =  m_widgets.size() - 1; x >= 1 ; x--) {
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

QFrame* LVSizeChart::buildFrame(QWidget *widget) 
{
    QFrame *const frame = new QFrame();
    frame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    frame->setLineWidth(2);

    QVBoxLayout *const layout = new QVBoxLayout();
    layout->addWidget(widget);
    layout->setSpacing(0);
    layout->setMargin(0);
    frame->setLayout(layout);

    return frame;
}
