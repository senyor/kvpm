/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
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
#include "pvtree.h"
#include "vginfolabels.h"
#include "vgremove.h"
#include "vgtree.h"
#include "volumegrouptab.h"


VolumeGroupTab::VolumeGroupTab(VolGroup *volumeGroup, QWidget *parent) : 
    QWidget(parent), m_vg(volumeGroup)
{

    m_group_name = m_vg->getName();
    
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    VGInfoLabels *vg_info_labels = new VGInfoLabels(m_vg);
    LVSizeChart  *lv_size_chart  = new LVSizeChart(m_vg);
    QSplitter *tree_splitter = new QSplitter(Qt::Vertical);
    QSplitter *lv_splitter = new QSplitter();
    QSplitter *pv_splitter = new QSplitter();

    layout->addWidget(vg_info_labels);
    layout->addWidget(lv_size_chart);
    layout->addWidget(tree_splitter);
    tree_splitter->addWidget(lv_splitter);
    tree_splitter->addWidget(pv_splitter);

    m_vg_tree = new VGTree(m_vg);
    m_pv_tree = new PVTree(m_vg);

    lv_splitter->addWidget(m_vg_tree);
    pv_splitter->addWidget(m_pv_tree);

    QWidget *test_widget = new QWidget;
    QLabel *test_label = new QLabel("volumes");
    QHBoxLayout *test_layout = new QHBoxLayout();
    test_widget->setBackgroundRole(QPalette::Base);
    test_widget->setAutoFillBackground(true);
    test_widget->setLayout(test_layout);
    test_layout->addWidget(test_label);
    pv_splitter->addWidget(test_widget);

    QScrollArea *lv_properties_scroll = new QScrollArea();
    lv_properties_scroll->setWidgetResizable(true);
    
    lv_properties_scroll->setBackgroundRole(QPalette::Base);
    lv_properties_scroll->setAutoFillBackground(true);

    m_lv_properties_stack = new LVPropertiesStack(m_vg);
    lv_properties_scroll->setWidget( m_lv_properties_stack );
    lv_splitter->addWidget(lv_properties_scroll);

    lv_splitter->setStretchFactor(0, 3);
    pv_splitter->setStretchFactor(0, 3);
    lv_splitter->setStretchFactor(1, 1);
    pv_splitter->setStretchFactor(1, 1);

    connect(m_vg_tree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), 
	    m_lv_properties_stack, SLOT(changeLVStackIndex(QTreeWidgetItem*, int)));
}

VolGroup* VolumeGroupTab::getVolumeGroup()
{
    return m_vg;   
}

QString VolumeGroupTab::getVolumeGroupName()
{
    return m_group_name;   // will return empty string if not a volume group
}

