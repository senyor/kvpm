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


#include "devicetab.h"

#include "devicetree.h"
#include "devicesizechart.h"
#include "deviceproperties.h"
#include "devicepropertiesstack.h"
#include "deviceactions.h"
#include "devicemenu.h"
#include "storagedevice.h"

#include <KConfigSkeleton>
#include <KLocale>
#include <KToolBar>

#include <QSplitter>
#include <QScrollArea>
#include <QVBoxLayout>



DeviceTab::DeviceTab(QWidget *parent) : 
    KMainWindow(parent)
{
    QWidget *const central = new QWidget();
    setCentralWidget(central);
    m_layout = new QVBoxLayout;
    central->setLayout(m_layout);

    m_size_chart   = new DeviceSizeChart(this);
    m_device_stack = new DevicePropertiesStack(this);
    m_tree = new DeviceTree(m_size_chart, m_device_stack, this);
    m_device_actions = new DeviceActions(this);
    addToolBar(buildDeviceToolBar());
    m_tree_properties_splitter = new QSplitter(Qt::Horizontal);

    m_layout->addWidget(m_size_chart);
    m_layout->addWidget(m_tree_properties_splitter);

    m_tree_properties_splitter->addWidget(m_tree);
    m_tree_properties_splitter->addWidget(setupPropertyStack());
    m_tree_properties_splitter->setStretchFactor(0, 9);
    m_tree_properties_splitter->setStretchFactor(1, 2);

    connect(m_tree, SIGNAL(deviceMenuRequested(QTreeWidgetItem*)),
            this, SLOT(deviceContextMenu(QTreeWidgetItem*)));

    connect(m_size_chart, SIGNAL(deviceMenuRequested(QTreeWidgetItem*)),
            this, SLOT(deviceContextMenu(QTreeWidgetItem*)));
}

void DeviceTab::rescan(QList<StorageDevice *> devices)
{
    disconnect(m_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
            m_device_actions, SLOT(changeDevice(QTreeWidgetItem*)));

    m_device_stack->loadData(devices);
    m_tree->loadData(devices);

    connect(m_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
            m_device_actions, SLOT(changeDevice(QTreeWidgetItem*)));

    m_device_actions->changeDevice(m_tree->currentItem());

    readConfig();
}

QScrollArea *DeviceTab::setupPropertyStack()
{
    QScrollArea *device_scroll = new QScrollArea();
    device_scroll->setFrameStyle(QFrame::NoFrame);
    device_scroll->setBackgroundRole(QPalette::Base);
    device_scroll->setAutoFillBackground(true);
    device_scroll->setWidget(m_device_stack);
    device_scroll->setWidgetResizable(true);

    return device_scroll;
}

void DeviceTab::deviceContextMenu(QTreeWidgetItem *item)
{
    m_device_actions->changeDevice(item);

    KMenu *menu = new DeviceMenu(m_device_actions, this);
    menu->exec(QCursor::pos());
    menu->deleteLater();
}

void DeviceTab::readConfig()
{
    KConfigSkeleton skeleton;
    bool show_toolbars, show_device_barchart;
    QString icon_size, icon_text;

    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("show_device_barchart", show_device_barchart, true);
    skeleton.addItemBool("show_toolbars", show_toolbars, true);
    skeleton.addItemString("toolbar_icon_size", icon_size, "mediumicons");
    skeleton.addItemString("toolbar_icon_text", icon_text, "iconsonly");

    if (show_device_barchart)
        m_size_chart->show();
    else
        m_size_chart->hide();

    if (show_toolbars)
        toolBar("devicetoolbar")->show();
    else
        toolBar("devicetoolbar")->hide();
    
    QSize size(22, 22);

    if (icon_size == "smallicons")
        size = QSize(16, 16);
    else if (icon_size == "mediumicons")
        size = QSize(22, 22);
    else if (icon_size == "largeicons")
        size = QSize(32, 32);
    else if (icon_size == "hugeicons")
        size = QSize(48, 48);

    toolBar("devicetoolbar")->setIconSize(size);
    
    if (icon_text == "iconsonly") 
        toolBar("devicetoolbar")->setToolButtonStyle(Qt::ToolButtonIconOnly);
    else if (icon_text == "textonly") 
        toolBar("devicetoolbar")->setToolButtonStyle(Qt::ToolButtonTextOnly);
    else if (icon_text == "textbesideicons") 
        toolBar("devicetoolbar")->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    else if (icon_text == "textundericons") 
        toolBar("devicetoolbar")->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
}

KToolBar* DeviceTab::buildDeviceToolBar()
{
    KToolBar *const toolbar = toolBar("devicetoolbar");
    toolbar->setContextMenuPolicy(Qt::PreventContextMenu);
    
    toolbar->addAction(m_device_actions->action("tablecreate"));
    toolbar->addSeparator();    
    toolbar->addAction(m_device_actions->action("partremove"));
    toolbar->addAction(m_device_actions->action("partadd"));
    toolbar->addAction(m_device_actions->action("partchange"));
    toolbar->addAction(m_device_actions->action("changeflags"));
    toolbar->addAction(m_device_actions->action("max_pv"));
    toolbar->addSeparator();    
    toolbar->addAction(m_device_actions->action("vgcreate"));
    toolbar->addAction(m_device_actions->action("vgreduce"));
    toolbar->addSeparator();    
    toolbar->addAction(m_device_actions->action("mount"));
    toolbar->addAction(m_device_actions->action("unmount"));
    toolbar->addSeparator();    
    toolbar->addAction(m_device_actions->action("max_fs"));
    toolbar->addAction(m_device_actions->action("fsck"));
    toolbar->addSeparator();    
    toolbar->addAction(m_device_actions->action("mkfs"));

    return toolbar;
}


