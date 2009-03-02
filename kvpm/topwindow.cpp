/*
 *
 * 
 * Copyright (C) 2008, 2009 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <KHelpMenu>
#include <KAboutData>
#include <KStandardAction>
#include <KLocale>
#include <QtGui>

#include "kvpmconfigdialog.h"
#include "devicetab.h"
#include "logvol.h"
#include "masterlist.h"
#include "physvol.h"
#include "processprogress.h"
#include "pvmove.h"
#include "removemissing.h"
#include "topwindow.h"
#include "vgcreate.h"
#include "vgchangealloc.h"
#include "vgchangeavailable.h"
#include "vgchangeextent.h"
#include "vgchangelv.h"
#include "vgchangepv.h"
#include "vgchangeresize.h"
#include "vgexport.h"
#include "vgimport.h"
#include "vgremove.h"
#include "vgrename.h"
#include "vgreduce.h"
#include "volgroup.h"
#include "volumegrouptab.h"

extern MasterList *master_list;


TopWindow::TopWindow(QWidget *parent):KMainWindow(parent)
{

    KMenu *file_menu     = new KMenu( i18n("File") );
    KMenu *tool_menu     = new KMenu( i18n("Tools") );
    KMenu *groups_menu   = new KMenu( i18n("Volume Groups") );
    KMenu *settings_menu = new KMenu( i18n("Settings") );


    KAboutData *about_data = new KAboutData( QByteArray("kvpm"),
					     QByteArray(""),
					     ki18n("kvpm"),
					     QByteArray("0.6.1"),
					     ki18n("Linux volume and partition manager for KDE.\n"
						   "This program is still under development,\n"
						   "bug reports and any comments are welcomed.\n"),
					     KAboutData::License_GPL_V3, 
					     ki18n("(c) 2008, 2009, Benjamin Scott"),
					     ki18n(""),
					     QByteArray("http://sourceforge.net/projects/kvpm/"),
					     QByteArray("benscott@nwlink.com") );


    KHelpMenu *help_menu = new KHelpMenu( this, about_data );

    menuBar()->addMenu(file_menu);
    menuBar()->addMenu(tool_menu);
    menuBar()->addMenu(groups_menu);
    menuBar()->addMenu(settings_menu);
    menuBar()->addMenu( help_menu->menu() );

    quit_action =  KStandardAction::quit(qApp, SLOT( quit() ), file_menu );

    remove_vg_action   = new KAction( KIcon("edit-delete"), i18n("Delete Volume Group..."), this);
    reduce_vg_action   = new KAction( i18n("Reduce Volume Group..."), this);
    rename_vg_action   = new KAction( KIcon("edit-rename"), i18n("Rename Volume Group..."), this);
    rescan_action      = new KAction( KIcon("view-refresh"), i18n("Rescan System"), this);
    rescan_vg_action   = new KAction( KIcon("view-refresh"), i18n("Rescan This Group"), this);
    m_restart_pvmove_action   = new KAction( KIcon("system-restart"), i18n("Restart interrupted pvmove"), this);
    m_stop_pvmove_action      = new KAction( KIcon("process-stop"), i18n("Abort pvmove"), this);
    remove_missing_action     = new KAction( i18n("Remove Missing Volumes..."), this);
    m_export_vg_action        = new KAction( i18n("Export Volume Group..."), this);
    m_import_vg_action        = new KAction( i18n("Import Volume Group..."), this);
    m_vgchange_menu           = new KMenu( i18n("Change Volume Group Attributes"), this);
    create_vg_action          = new KAction( i18n("Create Volume Group..."), this);
    vgchange_available_action = new KAction( i18n("Volume Group Availability..."), this);
    vgchange_alloc_action  = new KAction( i18n("Allocation Policy..."), this);
    vgchange_extent_action = new KAction( i18n("Extent Size..."), this);
    vgchange_lv_action     = new KAction( i18n("Logical Volume Limits..."), this);
    vgchange_pv_action     = new KAction( i18n("Physical Volume Limits..."), this);
    vgchange_resize_action = new KAction( i18n("Volume Group Resizability..."), this);
    m_vgchange_menu->addAction(vgchange_available_action);
    m_vgchange_menu->addAction(vgchange_alloc_action);
    m_vgchange_menu->addAction(vgchange_extent_action);
    m_vgchange_menu->addAction(vgchange_lv_action);
    m_vgchange_menu->addAction(vgchange_pv_action);
    m_vgchange_menu->addAction(vgchange_resize_action);

    config_kvpm_action     = new KAction( KIcon("configure"), i18n("Configure kvpm..."), this);

    file_menu->addAction(quit_action);
    tool_menu->addAction(rescan_action);
    tool_menu->addAction(rescan_vg_action);
    tool_menu->addSeparator();
    tool_menu->addAction(m_restart_pvmove_action);
    tool_menu->addAction(m_stop_pvmove_action);
    tool_menu->addSeparator();
    tool_menu->addAction(remove_missing_action);

    groups_menu->addAction(create_vg_action);
    groups_menu->addSeparator();
    groups_menu->addAction(remove_vg_action);
    groups_menu->addAction(reduce_vg_action);
    groups_menu->addAction(rename_vg_action);
    groups_menu->addAction(m_export_vg_action);
    groups_menu->addAction(m_import_vg_action);
    groups_menu->addMenu(m_vgchange_menu);

    settings_menu->addAction(config_kvpm_action);
    
    master_list = 0;
    m_tab_widget = 0;
    m_old_tab_widget =0;

    connect(vgchange_alloc_action,  SIGNAL(triggered()), 
	    this, SLOT(changeAllocation()));

    connect(vgchange_available_action,  SIGNAL(triggered()), 
	    this, SLOT(changeAvailable()));

    connect(vgchange_extent_action, SIGNAL(triggered()), 
	    this, SLOT(changeExtentSize()));

    connect(vgchange_lv_action,     SIGNAL(triggered()), 
	    this, SLOT(limitLogicalVolumes()));

    connect(vgchange_pv_action,     SIGNAL(triggered()), 
	    this, SLOT(limitPhysicalVolumes()));

    connect(vgchange_resize_action, SIGNAL(triggered()), 
	    this, SLOT(changeResize()));

    connect(create_vg_action,       SIGNAL(triggered()), 
	    this, SLOT(createVolumeGroup()));

    connect(remove_vg_action,       SIGNAL(triggered()), 
	    this, SLOT(removeVolumeGroup()));

    connect(rename_vg_action,       SIGNAL(triggered()), 
	    this, SLOT(renameVolumeGroup()));

    connect(m_export_vg_action,       SIGNAL(triggered()), 
	    this, SLOT(exportVolumeGroup()));

    connect(m_import_vg_action,       SIGNAL(triggered()), 
	    this, SLOT(importVolumeGroup()));

    connect(remove_missing_action,  SIGNAL(triggered()), 
	    this, SLOT(removeMissingVolumes()));

    connect(reduce_vg_action,       SIGNAL(triggered()), 
	    this, SLOT(reduceVolumeGroup()));

    connect(rescan_vg_action,       SIGNAL(triggered()), 
	    this, SLOT(rebuildVolumeGroupTab()));

    connect(rescan_action,          SIGNAL(triggered()), 
	    this, SLOT(reRun()));

    connect(m_restart_pvmove_action, SIGNAL(triggered()), 
	    this, SLOT(restartPhysicalVolumeMove()));

    connect(m_stop_pvmove_action,    SIGNAL(triggered()), 
	    this, SLOT(stopPhysicalVolumeMove()));

    connect(config_kvpm_action,    SIGNAL(triggered()), 
	    this, SLOT(configKvpm()));

    reRun();    // reRun also does the initial run
}

/* This destroys the central widget and its descendants
   then rescan_actions the system and puts up the information found */

void TopWindow::reRun()
{

    VolumeGroupTab *tab;
    QList<VolGroup *> groups;
    int vg_count;
    QString selected_vg_name;  // Name of the vg for the tab currently selected

    if( m_tab_widget ){
	
	if( m_tab_widget->currentIndex() > 0 )
	    selected_vg_name =  m_vg_tabs[m_tab_widget->currentIndex() - 1]->getVolumeGroupName();
	else
	    selected_vg_name =  "";
    }
    
    m_vg_tabs.clear();

    for(int x = 0; x < m_old_vg_tabs.size(); x++)
	delete(m_old_vg_tabs[x]);

    m_old_vg_tabs.clear();

    m_old_tab_widget = m_tab_widget;           // with this function we delay actually deleting

    if(m_old_tab_widget)                       // the widgets for one cycle.
	m_old_tab_widget->setParent(0);

    m_tab_widget = new KTabWidget(this);
    setCentralWidget(m_tab_widget);
    master_list = new MasterList();

    m_device_tab = new DeviceTab(master_list->getStorageDevices());
    m_tab_widget->addTab(m_device_tab, i18n("Storage devices") );

    vg_count = master_list->getVolGroupCount();
    groups = master_list->getVolGroups();

    m_tab_widget->setCurrentIndex(0);
    setupMenus(0);

    connect(m_tab_widget, SIGNAL(currentChanged(int)), 
	    this, SLOT(updateTabGeometry(int)));

    connect(m_tab_widget, SIGNAL(currentChanged(int)), 
	    this, SLOT(setupMenus(int)));

/* 
   Here we try to select the tab with the volume group
   that the user was looking at before the tabs were rebuilt.
   If that group no longer exists we go back to the first
   tab.
*/

    for(int x = 0; x < vg_count; x++){
	tab = new VolumeGroupTab(groups[x]);
	m_tab_widget->addTab(tab, groups[x]->getName());
	m_vg_tabs.append(tab);

	if( groups[x]->getName() == selected_vg_name ){
	    m_tab_widget->setCurrentIndex(x + 1);
	    setupMenus(x + 1);
	}
    }
}

/* This function destroys the displayed tab for the volume group at 
   the tab widget's current index and replaces it with a new one. It
   is called when something is changed in a volume group that doesn't
   effect the information on any other tab */


void TopWindow::rebuildVolumeGroupTab()
{
    int index = m_tab_widget->currentIndex();
    VolGroup *old_vg = m_vg_tabs[index - 1]->getVolumeGroup(); 
    QString group_name = old_vg->getName();

    m_vg = master_list->rebuildVolumeGroup(old_vg);
    VolumeGroupTab *tab = new VolumeGroupTab(m_vg);

    m_tab_widget->removeTab(index);
    m_tab_widget->insertTab(index, tab, group_name);

    m_old_vg_tabs.append( m_vg_tabs[index - 1] );
    
    m_vg_tabs.replace(index - 1, tab);

// These three lines must be last
    m_tab_widget->setCurrentIndex(index);
    m_vg_tabs[index - 1]->hide();
    m_vg_tabs[index - 1]->show();
}


void TopWindow::setupMenus(int index)
{
    index = m_tab_widget->currentIndex();

    if(index)
	m_vg = m_vg_tabs[index - 1]->getVolumeGroup();
    else 
	m_vg = NULL;

    if(m_vg){                                     // only enable group removal if the tab is
	if( !m_vg->getLogVolCount() )             // a volume group with no logical volumes
	    remove_vg_action->setEnabled(true);   
	else
	    remove_vg_action->setEnabled(false);   

	if( m_vg->isPartial() )
	    remove_missing_action->setEnabled(true);

	if( m_vg->isExported() ){
	    m_import_vg_action->setEnabled(true);
	    m_export_vg_action->setEnabled(false);
	}
	else{
	    m_import_vg_action->setEnabled(false);
	    m_export_vg_action->setEnabled(true);
	}
	
	reduce_vg_action->setEnabled(true);      // almost any group may be reduced
	rename_vg_action->setEnabled(true);      
	m_vgchange_menu->setEnabled(true);
	rescan_vg_action->setEnabled(true);
    }
    else{
	reduce_vg_action->setEnabled(false);
	rename_vg_action->setEnabled(false);      
	remove_vg_action->setEnabled(false);
	remove_missing_action->setEnabled(false);
	rescan_vg_action->setEnabled(false);
	m_vgchange_menu->setEnabled(false);
	m_import_vg_action->setEnabled(false);
	m_export_vg_action->setEnabled(false);
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
	m_vg_tabs[index -1]->hide();
	m_vg_tabs[index -1]->show();
    }
    else {
	m_device_tab->hide();
	m_device_tab->show();
    }
}

void TopWindow::changeAllocation()
{
    VGChangeAllocDialog dialog( m_vg->getName() );
    dialog.exec();

    if(dialog.result() == QDialog::Accepted){
        ProcessProgress change_vg(dialog.arguments(), "", false);
	MainWindow->rebuildVolumeGroupTab();
    }
}

void TopWindow::changeAvailable()
{
    if( change_vg_available(m_vg) )
	MainWindow->rebuildVolumeGroupTab();
}

void TopWindow::changeExtentSize()
{
    if( change_vg_extent(m_vg) )
	MainWindow->rebuildVolumeGroupTab();
}

void TopWindow::limitLogicalVolumes()
{
    if( change_vg_lv(m_vg) )
	MainWindow->rebuildVolumeGroupTab();
}

void TopWindow::limitPhysicalVolumes()
{
    if( change_vg_pv(m_vg) )
	MainWindow->rebuildVolumeGroupTab();
}

void TopWindow::changeResize()
{
    if( change_vg_resize(m_vg) )
	MainWindow->rebuildVolumeGroupTab();
}

void TopWindow::createVolumeGroup()
{
    if( create_vg() )
        MainWindow->reRun();
}

void TopWindow::removeVolumeGroup()
{
    if( remove_vg(m_vg) )
        MainWindow->reRun();
}

void TopWindow::renameVolumeGroup()
{
    if( rename_vg(m_vg) )
        MainWindow->reRun();
}

void TopWindow::reduceVolumeGroup()
{
    if( reduce_vg(m_vg) )
        MainWindow->reRun();
}

void TopWindow::exportVolumeGroup()
{
    if( export_vg(m_vg) )
        MainWindow->reRun();
}

void TopWindow::importVolumeGroup()
{
    if( import_vg(m_vg) )
        MainWindow->reRun();
}

void TopWindow::removeMissingVolumes()
{
    if( remove_missing_pv(m_vg) )
        MainWindow->reRun();
}

void TopWindow::restartPhysicalVolumeMove()
{
    if( restart_pvmove() )
        MainWindow->reRun();
}

void TopWindow::stopPhysicalVolumeMove()
{
    if( stop_pvmove() )
        MainWindow->reRun();
}

void TopWindow::configKvpm()
{
    if( config_kvpm() )
        MainWindow->reRun();
}
