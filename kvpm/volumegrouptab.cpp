/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "volumegrouptab.h"

#include <KConfigSkeleton>

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


VolumeGroupTab::VolumeGroupTab(VolGroup *volumeGroup, QWidget *parent) : QWidget(parent), m_vg(volumeGroup) 
{
    QSplitter *const tree_splitter = new QSplitter(Qt::Vertical);
    QSplitter *const lv_splitter   = new QSplitter();
    QSplitter *const pv_splitter   = new QSplitter();
    m_vg_info_labels = NULL;
    m_lv_size_chart  = NULL;
    m_lv_properties_stack = NULL;
    m_pv_properties_stack = NULL;

    m_layout = new QVBoxLayout;
    setLayout(m_layout);
    m_group_name = m_vg->getName();

    m_vg_tree = new VGTree(m_vg);
    m_pv_tree = new PVTree(m_vg);
    m_vg_tree->setAlternatingRowColors(true);
    m_pv_tree->setAlternatingRowColors(true);

    m_layout->addWidget(tree_splitter);
    tree_splitter->addWidget(lv_splitter);
    tree_splitter->addWidget(pv_splitter);
    lv_splitter->addWidget(m_vg_tree);
    pv_splitter->addWidget(m_pv_tree);

    tree_splitter->setStretchFactor(0, 3); 
    tree_splitter->setStretchFactor(1, 2); 

    QList<int> lv_size_list;
    lv_size_list << 1500 << 10 ;
    lv_splitter->setSizes( lv_size_list );
    m_lv_properties_stack = new LVPropertiesStack(m_vg);
    m_lv_properties_stack->setFrameStyle( m_vg_tree->frameStyle() );
    lv_splitter->addWidget(m_lv_properties_stack);

    QList<int> pv_size_list;
    pv_size_list << 1500 << 10 ;
    pv_splitter->setSizes( pv_size_list );
    m_pv_properties_stack = new PVPropertiesStack(m_vg);
    m_pv_properties_stack->setFrameStyle( m_vg_tree->frameStyle() );
    pv_splitter->addWidget(m_pv_properties_stack);

    return;
}

void VolumeGroupTab::rescan()
{
    disconnect(m_vg_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), 
               m_lv_properties_stack, SLOT(changeLVStackIndex(QTreeWidgetItem*, QTreeWidgetItem*)));

    disconnect(m_pv_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), 
               m_pv_properties_stack, SLOT(changePVStackIndex(QTreeWidgetItem*, QTreeWidgetItem*)));

    if(m_vg_info_labels){
        m_layout->removeWidget(m_vg_info_labels);
        m_vg_info_labels->setParent(NULL);
        m_vg_info_labels->deleteLater();
    }
    m_vg_info_labels = new VGInfoLabels(m_vg);
    m_layout->insertWidget(0, m_vg_info_labels);

    m_lv_properties_stack->loadData();
    m_pv_properties_stack->loadData();

    connect(m_vg_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), 
	    m_lv_properties_stack, SLOT(changeLVStackIndex(QTreeWidgetItem*, QTreeWidgetItem*)));

    connect(m_pv_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), 
	    m_pv_properties_stack, SLOT(changePVStackIndex(QTreeWidgetItem*, QTreeWidgetItem*)));

    m_vg_tree->loadData(); // This needs to be done after the lv property stack is built
    m_pv_tree->loadData(); // This needs to be done after the pv property stack is loaded

    if(m_lv_size_chart){   // This needs to be after the vgtree is loaded
        m_layout->removeWidget(m_lv_size_chart);
        m_lv_size_chart->setParent(NULL);
        m_lv_size_chart->deleteLater();
    }
    m_lv_size_chart  = new LVSizeChart(m_vg, m_vg_tree);
    m_layout->insertWidget(1, m_lv_size_chart);

    readConfig();

    return;
}

VolGroup* VolumeGroupTab::getVg()
{
    return m_vg;   
}

void VolumeGroupTab::readConfig()
{
    KConfigSkeleton skeleton;
    bool show_vg_info, show_lv_bar;

    skeleton.setCurrentGroup("General");
    skeleton.addItemBool( "show_vg_info", show_vg_info );
    skeleton.addItemBool( "show_lv_bar",  show_lv_bar );

    if(show_vg_info)
        m_vg_info_labels->show();
    else
        m_vg_info_labels->hide();

    if(show_lv_bar)
        m_lv_size_chart->show();
    else
        m_lv_size_chart->hide();
}



