/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
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

#include <QDebug>
#include <QFrame>
#include <QLabel>
#include <QScrollArea>
#include <QSplitter>
#include <QString>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include <KActionCollection>
#include <KComponentData>
#include <KConfigSkeleton>
#include <KGlobal>
#include <KLocale>
#include <KSeparator>
#include <KSharedConfig>
#include <KToolBar>

#include "lvpropertiesstack.h"
#include "lvsizechart.h"
#include "physvol.h"
#include "pvpropertiesstack.h"
#include "pvtree.h"
#include "vginfolabels.h"
#include "vgremove.h"
#include "vgtree.h"
#include "volgroup.h"
#include "lvactions.h"
#include "lvactionsmenu.h"
#include "pvactions.h"
#include "pvactionsmenu.h"


VolumeGroupTab::VolumeGroupTab(VolGroup *const group, QWidget *parent) : 
    KMainWindow(parent),
    m_vg(group)
{
    QWidget *const central = new QWidget();
    setCentralWidget(central);
    m_layout = new QVBoxLayout;
    central->setLayout(m_layout);

    QSplitter *const tree_splitter = new QSplitter(Qt::Vertical);
    QSplitter *const lv_splitter   = new QSplitter();
    QSplitter *const pv_splitter   = new QSplitter();
    m_lv_actions = new LVActions(group, this);
    m_pv_actions = new PVActions(group, this);

    addToolBar(buildLvToolBar());
    addToolBar(buildPvToolBar());

    m_vg_tree = new VGTree(m_vg);
    m_pv_tree = new PVTree(m_vg);

    connect(m_vg_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
            m_lv_actions, SLOT(changeLv(QTreeWidgetItem*)));

    connect(m_vg_tree, SIGNAL(lvMenuRequested(QTreeWidgetItem*)),
            this, SLOT(lvContextMenu(QTreeWidgetItem*)));

    connect(m_pv_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
            m_pv_actions, SLOT(changePv(QTreeWidgetItem*)));

    connect(m_pv_tree, SIGNAL(pvMenuRequested(QTreeWidgetItem*)),
            this, SLOT(pvContextMenu(QTreeWidgetItem*)));

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
    lv_splitter->setSizes(lv_size_list);
    m_lv_properties_stack = new LVPropertiesStack(m_vg);
    m_lv_properties_stack->setFrameStyle(m_vg_tree->frameStyle());
    lv_splitter->addWidget(m_lv_properties_stack);

    QList<int> pv_size_list;
    pv_size_list << 1500 << 10 ;
    pv_splitter->setSizes(pv_size_list);
    m_pv_properties_stack = new PVPropertiesStack(m_vg);
    m_pv_properties_stack->setFrameStyle(m_vg_tree->frameStyle());
    pv_splitter->addWidget(m_pv_properties_stack);

    return;
}

void VolumeGroupTab::rescan()
{
    disconnect(m_vg_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
               m_lv_properties_stack, SLOT(changeLVStackIndex(QTreeWidgetItem*, QTreeWidgetItem*)));

    disconnect(m_pv_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
               m_pv_properties_stack, SLOT(changePVStackIndex(QTreeWidgetItem*, QTreeWidgetItem*)));

    if (m_vg_info_labels) {
        m_layout->removeWidget(m_vg_info_labels);
        m_vg_info_labels->setParent(nullptr);
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

    if (m_partial_warning) {
        m_layout->removeWidget(m_partial_warning);
        m_partial_warning->setParent(nullptr);
        m_partial_warning->deleteLater();
    }
    m_partial_warning  = buildPartialWarning();
    m_layout->insertWidget(1, m_partial_warning);
    if (m_vg->isPartial())
        m_partial_warning->show();
    else
        m_partial_warning->hide();

    if (m_lv_size_chart) { // This needs to be after the vgtree is loaded
        m_layout->removeWidget(m_lv_size_chart);
        m_lv_size_chart->setParent(nullptr);
        m_lv_size_chart->deleteLater();
    }
    m_lv_size_chart  = new LVSizeChart(m_vg, m_vg_tree);
    m_layout->insertWidget(2, m_lv_size_chart);

    connect(m_lv_size_chart, SIGNAL(lvMenuRequested(LogVol*)),
            this, SLOT(lvContextMenu(LogVol*)));

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
    bool show_vg_info, show_lv_bar, show_toolbars;
    QString icon_size, icon_text;

    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("show_vg_info",  show_vg_info,  true);
    skeleton.addItemBool("show_lv_bar",   show_lv_bar,   true);
    skeleton.addItemBool("show_toolbars", show_toolbars, true);
    skeleton.addItemString("toolbar_icon_size", icon_size, "mediumicons");
    skeleton.addItemString("toolbar_icon_text", icon_text, "iconsonly");

    if (show_vg_info)
        m_vg_info_labels->show();
    else
        m_vg_info_labels->hide();

    if (show_lv_bar)
        m_lv_size_chart->show();
    else
        m_lv_size_chart->hide();

    if (show_toolbars) {
        toolBar("lvtoolbar")->show();
        toolBar("pvtoolbar")->show();
    } else {
        toolBar("lvtoolbar")->hide();
        toolBar("pvtoolbar")->hide();
    }

    QSize size(22, 22);

    if (icon_size == "smallicons")
        size = QSize(16, 16);
    else if (icon_size == "mediumicons")
        size = QSize(22, 22);
    else if (icon_size == "largeicons")
        size = QSize(32, 32);
    else if (icon_size == "hugeicons")
        size = QSize(48, 48);

    toolBar("lvtoolbar")->setIconSize(size);
    toolBar("pvtoolbar")->setIconSize(size);
    
    if (icon_text == "iconsonly") {
        toolBar("lvtoolbar")->setToolButtonStyle(Qt::ToolButtonIconOnly);
        toolBar("pvtoolbar")->setToolButtonStyle(Qt::ToolButtonIconOnly);
    } else if (icon_text == "textonly") {
        toolBar("lvtoolbar")->setToolButtonStyle(Qt::ToolButtonTextOnly);
        toolBar("pvtoolbar")->setToolButtonStyle(Qt::ToolButtonTextOnly);
    } else if (icon_text == "textbesideicons") {
        toolBar("lvtoolbar")->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        toolBar("pvtoolbar")->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    } else if (icon_text == "textundericons") {
        toolBar("lvtoolbar")->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        toolBar("pvtoolbar")->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }
}

QFrame* VolumeGroupTab::buildPartialWarning()
{
    QFrame *const warning = new QFrame();
    warning->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    warning->setLineWidth(2);

    QHBoxLayout *const layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->setMargin(0);
    QLabel *const label = new QLabel(i18n("<b>Warning: Partial volume group, some physical volumes are missing</b>"));
    label->setBackgroundRole(QPalette::Base);
    label->setAutoFillBackground(true);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    warning->setLayout(layout);

    return warning;
}

KToolBar* VolumeGroupTab::buildLvToolBar()
{
    KToolBar *const toolbar = toolBar("lvtoolbar");
    toolbar->setContextMenuEnabled(false);

    toolbar->addAction(m_lv_actions->action("lvcreate"));
    toolbar->addAction(m_lv_actions->action("thinpool"));
    toolbar->addAction(m_lv_actions->action("thincreate"));
    toolbar->addSeparator();    
    toolbar->addAction(m_lv_actions->action("lvremove"));
    toolbar->addSeparator();    
    toolbar->addAction(m_lv_actions->action("lvrename"));
    toolbar->addAction(m_lv_actions->action("snapcreate"));
    toolbar->addAction(m_lv_actions->action("thinsnap"));
    toolbar->addAction(m_lv_actions->action("snapmerge"));
    toolbar->addAction(m_lv_actions->action("lvreduce"));
    toolbar->addAction(m_lv_actions->action("lvextend"));
    toolbar->addAction(m_lv_actions->action("pvmove"));
    toolbar->addAction(m_lv_actions->action("lvchange"));
    toolbar->addSeparator();    
    toolbar->addAction(m_lv_actions->action("addlegs"));
    toolbar->addAction(m_lv_actions->action("changelog"));
    toolbar->addAction(m_lv_actions->action("removemirror"));
    toolbar->addAction(m_lv_actions->action("removethis"));
    toolbar->addAction(m_lv_actions->action("repairmissing"));
    toolbar->addAction(m_lv_actions->action("mount"));
    toolbar->addAction(m_lv_actions->action("unmount"));
    toolbar->addSeparator();    
    toolbar->addAction(m_lv_actions->action("maxfs"));
    toolbar->addAction(m_lv_actions->action("fsck"));
    toolbar->addAction(m_lv_actions->action("mkfs"));

    return toolbar;
}

KToolBar* VolumeGroupTab::buildPvToolBar()
{
    KToolBar *const toolbar = toolBar("pvtoolbar");
    toolbar->setContextMenuEnabled(false);

    KSeparator *const separator = new KSeparator(Qt::Vertical);
    separator->setMinimumWidth(40);
    toolbar->addWidget(separator);

    toolbar->addAction(m_pv_actions->action("pvmove"));
    toolbar->addAction(m_pv_actions->action("pvremove"));
    toolbar->addAction(m_pv_actions->action("pvchange"));

    return toolbar;
}

void VolumeGroupTab::lvContextMenu(LogVol *lv)
{
    m_lv_actions->changeLv(lv, -1);
    
    KMenu *menu = new LVActionsMenu(m_lv_actions, this);
    menu->exec(QCursor::pos());
    menu->deleteLater();
}

void VolumeGroupTab::lvContextMenu(QTreeWidgetItem *item)
{
    m_lv_actions->changeLv(item);

    KMenu *menu = new LVActionsMenu(m_lv_actions, this);
    menu->exec(QCursor::pos());
    menu->deleteLater();
}

void VolumeGroupTab::pvContextMenu(QTreeWidgetItem *item)
{
    m_pv_actions->changePv(item);

    KMenu *menu = new PVActionsMenu(m_pv_actions, this);
    menu->exec(QCursor::pos());
    menu->deleteLater();
}
