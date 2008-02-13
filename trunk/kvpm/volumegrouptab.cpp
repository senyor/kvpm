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


#include <QtGui>

#include "lvpropertiesstack.h"
#include "lvsizechart.h"
#include "vginfolabels.h"
#include "vgremove.h"
#include "volumegrouptab.h"


VolumeGroupTab::VolumeGroupTab(VolGroup *VolumeGroup, QWidget *parent) : 
    QWidget(parent), vg(VolumeGroup)
{

    group_name = vg->getName();
    
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    VGInfoLabels *vg_info_labels = new VGInfoLabels(vg);
    layout->addWidget(vg_info_labels);

    LVSizeChart *lv_size_chart = new LVSizeChart(vg);
    layout->addWidget(lv_size_chart);

    vg_tree = new VGTree(vg);
    layout->addWidget(vg_tree);

    QWidget *pv_tree_widget = new QWidget();
    
    QVBoxLayout *pv_tree_layout = new QVBoxLayout;
    pv_tree_layout->setMargin(0);
    
    pv_tree_widget->setLayout(pv_tree_layout);
    QLabel *pv_label = new QLabel("<b>Physical Volumes</b>");
    pv_tree = new PVTree(vg->getPhysicalVolumes());

    QSplitter *splitter = new QSplitter();
    
    layout->addWidget(splitter);
    pv_tree_layout->addWidget(pv_label, 0, Qt::AlignHCenter);
    pv_tree_layout->addWidget(pv_tree);
    splitter->addWidget(pv_tree_widget);
    splitter->setStretchFactor(0, 1);

    QWidget *lv_properties_widget = new QWidget();
    QVBoxLayout *lv_properties_layout = new QVBoxLayout;
    lv_properties_layout->setMargin(0);
    
    lv_properties_widget->setLayout(lv_properties_layout);

    QLabel *lv_properties_label = new QLabel("<b>Logical Volume Properties</b>");
    lv_properties_layout->addWidget(lv_properties_label, 0 , Qt::AlignHCenter);

    QScrollArea *lv_properties_scroll = new QScrollArea();
    lv_properties_layout->addWidget(lv_properties_scroll);
    lv_properties_stack = new LVPropertiesStack(vg);
    lv_properties_scroll->setWidget( lv_properties_stack );

    splitter->addWidget(lv_properties_widget);

    connect(vg_tree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), 
	    lv_properties_stack, SLOT(changeLVStackIndex(QTreeWidgetItem*, int)));
}

VolGroup* VolumeGroupTab::getVolumeGroup()
{
    return vg;   
}

QString VolumeGroupTab::getVolumeGroupName()
{
    return group_name;   // will return empty string if not a volume group
}

