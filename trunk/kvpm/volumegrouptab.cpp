/*
 *
 * 
 * Copyright (C) 2008, 2010 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <QtGui>

#include "lvpropertiesstack.h"
#include "lvsizechart.h"
#include "physvol.h"
#include "pvpropertiesstack.h"
#include "pvtree.h"
#include "vginfolabels.h"
#include "vgremove.h"
#include "vgtree.h"
#include "volgroup.h"
#include "volumegrouptab.h"


VolumeGroupTab::VolumeGroupTab(VolGroup *volumeGroup, QWidget *parent) : QWidget(parent), m_vg(volumeGroup) 
{
    m_layout = new QVBoxLayout;
    setLayout(m_layout);
    m_visable_widget = NULL;
    m_group_name = m_vg->getName();

    m_vg_tree = new VGTree(m_vg);
    m_pv_tree = new PVTree(m_vg);
    m_vg_tree->setAlternatingRowColors(true);
    m_pv_tree->setAlternatingRowColors(true);

    return;
}

void VolumeGroupTab::rescan()
{
    m_vg_tree->setParent(NULL);
    m_pv_tree->setParent(NULL);

    disconnect(m_vg_tree, 0, m_lv_properties_stack, 0);
    disconnect(m_pv_tree, 0, m_pv_properties_stack, 0);

    if(m_visable_widget){
        m_layout->removeWidget(m_visable_widget);
        m_visable_widget->setParent(0);
        m_visable_widget->deleteLater();
    }

    QVBoxLayout *visable_layout = new QVBoxLayout;
    m_visable_widget = new QWidget;
    m_visable_widget->setLayout(visable_layout);
    m_layout->addWidget(m_visable_widget);

    VGInfoLabels *vg_info_labels = new VGInfoLabels(m_vg);
    LVSizeChart  *lv_size_chart  = new LVSizeChart(m_vg);
    QSplitter *tree_splitter = new QSplitter(Qt::Vertical);
    QSplitter *lv_splitter   = new QSplitter();
    QSplitter *pv_splitter   = new QSplitter();

    visable_layout->addWidget(vg_info_labels);
    visable_layout->addWidget(lv_size_chart);
    visable_layout->addWidget(tree_splitter);
    tree_splitter->addWidget(lv_splitter);
    tree_splitter->addWidget(pv_splitter);

    lv_splitter->addWidget(m_vg_tree);
    pv_splitter->addWidget(m_pv_tree);

    QScrollArea *lv_properties_scroll = new QScrollArea();
    lv_properties_scroll->setWidgetResizable(true);
    lv_properties_scroll->setFrameStyle(QFrame::NoFrame);
    lv_properties_scroll->setBackgroundRole(QPalette::Base);
    lv_properties_scroll->setAutoFillBackground(true);

    m_lv_properties_stack = new LVPropertiesStack(m_vg);
    m_pv_properties_stack = new PVPropertiesStack(m_vg);
    lv_properties_scroll->setWidget(m_lv_properties_stack);
    lv_splitter->addWidget(lv_properties_scroll);
    pv_splitter->addWidget(m_pv_properties_stack);

    lv_splitter->setStretchFactor(0, 3);
    lv_splitter->setStretchFactor(1, 1);
    pv_splitter->setStretchFactor(0, 7);
    pv_splitter->setStretchFactor(1, 3);

    connect(m_vg_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), 
	    m_lv_properties_stack, SLOT(changeLVStackIndex(QTreeWidgetItem*, QTreeWidgetItem*)));

    connect(m_pv_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), 
	    m_pv_properties_stack, SLOT(changePVStackIndex(QTreeWidgetItem*, QTreeWidgetItem*)));

    m_vg_tree->loadData(); // This needs to be done after the lv property stack is built
    m_pv_tree->loadData(); // This needs to be done after the pv property stack is built

    return;
}

VolGroup* VolumeGroupTab::getVolumeGroup()
{
    return m_vg;   
}

QString VolumeGroupTab::getVolumeGroupName()
{
    return m_group_name;   // will return empty string if not a volume group
}

