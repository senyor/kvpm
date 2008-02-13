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


#include <KStandardAction>
#include <QtGui>
#include "devicetab.h"
#include "logvol.h"
#include "masterlist.h"
#include "physvol.h"
#include "processprogress.h"
#include "removemissing.h"
#include "topwindow.h"
#include "vgchangealloc.h"
#include "vgchangeextent.h"
#include "vgchangelv.h"
#include "vgchangepv.h"
#include "vgchangeresize.h"
#include "vgremove.h"
#include "vgreduce.h"
#include "volgroup.h"
#include "volumegrouptab.h"

extern MasterList *master_list;


TopWindow::TopWindow(QWidget *parent):KMainWindow(parent)
{

    file_menu     = new KMenu("File");
    tool_menu     = new KMenu("Tools");
    groups_menu   = new KMenu("Volume Groups");
    settings_menu = new KMenu("Settings");
    
    help_menu   = helpMenu( "Linux volume and partition manager for KDE" );

    menuBar()->addMenu(file_menu);
    menuBar()->addMenu(tool_menu);
    menuBar()->addMenu(groups_menu);
    menuBar()->addMenu(settings_menu);
    menuBar()->addMenu(help_menu);

    quit_action =  KStandardAction::quit(qApp, SLOT( quit() ), file_menu );

    remove_vg_action   = new KAction( KIcon("edit-delete"), "Delete Volume Group...", this);
    reduce_vg_action   = new KAction("Reduce Volume Group...", this);
    rescan_action      = new KAction( KIcon("rebuild"), "Rescan System", this);
    rescan_vg_action   = new KAction( KIcon("rebuild"), "Rescan This Group", this);

    remove_missing_action =  new KAction("Remove Missing Volumes...", this);
    vgchange_menu   = new KMenu("Change Volume Group Attributes", this);
    vgchange_alloc_action  = new KAction("Allocation Policy...", this);
    vgchange_extent_action = new KAction("Extent Size...", this);
    vgchange_lv_action     = new KAction("Logical Volume Limits...", this);
    vgchange_pv_action     = new KAction("Physical Volume Limits...", this);
    vgchange_resize_action = new KAction("Volume Group Resizability...", this);
    config_kvpm_action     = new KAction("Configure kvpm...", this);
    vgchange_menu->addAction(vgchange_alloc_action);
    vgchange_menu->addAction(vgchange_extent_action);
    vgchange_menu->addAction(vgchange_lv_action);
    vgchange_menu->addAction(vgchange_pv_action);
    vgchange_menu->addAction(vgchange_resize_action);

    file_menu->addAction(quit_action);
    tool_menu->addAction(rescan_action);
    tool_menu->addAction(rescan_vg_action);
    tool_menu->addAction(remove_missing_action);
    groups_menu->addMenu(vgchange_menu);
    groups_menu->addAction(remove_vg_action);
    groups_menu->addAction(reduce_vg_action);
    settings_menu->addAction(config_kvpm_action);
    
    master_list = 0;
    tab_widget = 0;
    old_tab_widget =0;

    connect(vgchange_alloc_action,  SIGNAL(triggered()), this, SLOT(launchVGChangeAllocDialog()));
    connect(vgchange_extent_action, SIGNAL(triggered()), this, SLOT(launchVGChangeExtentDialog()));
    connect(vgchange_lv_action,     SIGNAL(triggered()), this, SLOT(launchVGChangeLVDialog()));
    connect(vgchange_pv_action,     SIGNAL(triggered()), this, SLOT(launchVGChangePVDialog()));
    connect(vgchange_resize_action, SIGNAL(triggered()), this, SLOT(launchVGChangeResizeDialog()));
    connect(remove_vg_action,       SIGNAL(triggered()), this, SLOT(launchVGRemoveDialog()));
    connect(remove_missing_action,  SIGNAL(triggered()), this, SLOT(launchRemoveMissingDialog()));
    connect(reduce_vg_action,       SIGNAL(triggered()), this, SLOT(launchVGReduceDialog()));
    connect(rescan_vg_action,       SIGNAL(triggered()), this, SLOT(rebuildVolumeGroupTab()));
    connect(rescan_action,          SIGNAL(triggered()), this, SLOT(reRun()));

    reRun();    // reRun also does the initial run
}

/* This destroys the central widget and its descendants
   then rescan_actions the system and puts up the information found */

void TopWindow::reRun()
{
    VolumeGroupTab *tab;
    QList<VolGroup *> groups;
    int vg_count;
    vg_tabs.clear();
    old_tab_widget = tab_widget;           // with this function we delay actually deleting
    if(old_tab_widget)                     // the widgets for one cycle.
	old_tab_widget->setParent(0);

    tab_widget = new KTabWidget(this);
    setCentralWidget(tab_widget);
    master_list = new MasterList();

    device_tab = new DeviceTab(master_list->getStorageDevices());
    tab_widget->addTab(device_tab, "storage devices");

    vg_count = master_list->getVolGroupCount();
    groups = master_list->getVolGroups();
    for(int x = 0; x < vg_count; x++){
	tab = new VolumeGroupTab(groups[x]);
	tab_widget->addTab(tab, groups[x]->getName());
	vg_tabs.append(tab);
    }
    
    tab_widget->setCurrentIndex(0);
    setupMenus(0);

    connect(tab_widget, SIGNAL(currentChanged(int)), this, SLOT(updateTabGeometry(int)));
    connect(tab_widget, SIGNAL(currentChanged(int)), this, SLOT(setupMenus(int)));
}

void TopWindow::rebuildVolumeGroupTab()
{
    VolGroup *vg;

    vg = vg_tabs[(tab_widget->currentIndex()) - 1]->getVolumeGroup();

    MainWindow->rebuildVolumeGroupTab(vg);
}


void TopWindow::rebuildVolumeGroupTab(VolGroup *VolumeGroup)
{
    VolGroup *old_vg, *new_vg;
    QString group_name;
    VolumeGroupTab *tab;
    int index = 0;

    old_vg = VolumeGroup;
    group_name = old_vg->getName();

    while( (index < vg_tabs.size()) && (vg_tabs[index]->getVolumeGroupName() != group_name) )
	index++;
    
    master_list->rebuildVolumeGroup(old_vg);
    new_vg = master_list->getVolGroupByName(group_name);

    tab = new VolumeGroupTab(new_vg);
    tab_widget->removeTab(index + 1);
    tab_widget->insertTab(index + 1, tab, new_vg->getName());
    tab_widget->setCurrentIndex(index + 1);
    delete vg_tabs[index];
    
    vg_tabs.replace(index, tab);
    vg_tabs[index]->hide();
    vg_tabs[index]->show();
}


void TopWindow::setupMenus(int index)
{
    index = tab_widget->currentIndex();
    
    if(index)
	vg = vg_tabs[index - 1]->getVolumeGroup();
    else 
	vg = NULL;

    if(vg){                                   // only enable group removal if the tab is
	if(!vg->getLogVolCount())             // a volume group with no logical volumes
	    remove_vg_action->setEnabled(TRUE);   
	if(vg->isPartial())
	    remove_missing_action->setEnabled(TRUE);
	reduce_vg_action->setEnabled(TRUE);      // almost any group may be reduced
	vgchange_menu->setEnabled(TRUE);
	rescan_vg_action->setEnabled(TRUE);
    }
    else{
	reduce_vg_action->setEnabled(FALSE);
	remove_vg_action->setEnabled(FALSE);
	remove_missing_action->setEnabled(FALSE);
	rescan_vg_action->setEnabled(FALSE);
	vgchange_menu->setEnabled(FALSE);
    }
}

/*
   The next function works around what may be a Qt bug. The first
   time a size chart is shown on a tab the geometry is wrong. Hiding
   and then showing the tab causes the geometry to get redone correctly
*/

void TopWindow::updateTabGeometry(int index)
{
    if(index){
	vg_tabs[index -1]->hide();
	vg_tabs[index -1]->show();
    }
    else {
	device_tab->hide();
	device_tab->show();
    }
}

void TopWindow::launchVGChangeAllocDialog()
{
    VGChangeAllocDialog dialog( vg->getName() );
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress change_vg(dialog.arguments(), "", FALSE);
	MainWindow->rebuildVolumeGroupTab(vg);
    }
}

void TopWindow::launchVGChangeExtentDialog()
{
    if( change_vg_extent(vg) )
	MainWindow->rebuildVolumeGroupTab(vg);
}

void TopWindow::launchVGChangeLVDialog()
{
    if( change_vg_lv(vg) )
	MainWindow->rebuildVolumeGroupTab(vg);
}

void TopWindow::launchVGChangePVDialog()
{
    if( change_vg_pv(vg) )
	MainWindow->rebuildVolumeGroupTab(vg);
}

void TopWindow::launchVGChangeResizeDialog()
{
    if( change_vg_resize(vg) )
	MainWindow->rebuildVolumeGroupTab(vg);
}

void TopWindow::launchVGRemoveDialog()
{
    if( remove_vg(vg) )
        MainWindow->reRun();
}

void TopWindow::launchVGReduceDialog()
{
    if( reduce_vg(vg) )
        MainWindow->reRun();
}

void TopWindow::launchRemoveMissingDialog()
{
    if( remove_missing_pv(vg) )
        MainWindow->reRun();
}

