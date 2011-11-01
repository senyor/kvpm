/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "topwindow.h"

#include <KAboutData>
#include <KHelpMenu>
#include <KIcon>
#include <KLocale>
#include <KStandardAction>

#include <QtGui>

#include "kvpmconfigdialog.h"
#include "devicetab.h"
#include "logvol.h"
#include "masterlist.h"
#include "maintabwidget.h"
#include "physvol.h"
#include "processprogress.h"
#include "pvmove.h"
#include "removemissing.h"
#include "vgcreate.h"
#include "vgchange.h"
#include "vgexport.h"
#include "vgextend.h"
#include "vgimport.h"
#include "vgmerge.h"
#include "vgremove.h"
#include "vgrename.h"
#include "vgreduce.h"
#include "vgsplit.h"
#include "volgroup.h"
#include "volumegrouptab.h"

extern MasterList *g_master_list;


TopWindow::TopWindow(QWidget *parent):KMainWindow(parent)
{

    KMenu *file_menu     = new KMenu( i18n("File") );
    KMenu *tool_menu     = new KMenu( i18n("Tools") );
    KMenu *groups_menu   = new KMenu( i18n("Volume Groups") );
    KMenu *settings_menu = new KMenu( i18n("Settings") );


    KAboutData *about_data = new KAboutData( QByteArray("kvpm"),
					     QByteArray(""),
					     ki18n("kvpm"),
					     QByteArray("0.8.2"),
					     ki18n("Linux volume and partition manager for KDE.\n"
						   "This program is still under development,\n"
						   "bug reports and any comments are welcomed.\n"),
					     KAboutData::License_GPL_V3, 
					     ki18n("(c) 2008, 2009, 2010, 2011 Benjamin Scott"),
					     ki18n(" "),
					     QByteArray("http://sourceforge.net/projects/kvpm/"),
					     QByteArray("benscott@nwlink.com") );


    KHelpMenu *help_menu = new KHelpMenu( this, about_data );

    menuBar()->addMenu(file_menu);
    menuBar()->addMenu(tool_menu);
    menuBar()->addMenu(groups_menu);
    menuBar()->addMenu(settings_menu);
    menuBar()->addMenu( help_menu->menu() );

    quit_action =  KStandardAction::quit(qApp, SLOT( quit() ), file_menu );

    remove_vg_action       = new KAction( KIcon("cross"),        i18n("Delete Volume Group..."), this);
    reduce_vg_action       = new KAction( KIcon("delete"),       i18n("Reduce Volume Group..."), this);
    extend_vg_action       = new KAction( KIcon("add"),          i18n("Extend Volume Group..."), this);
    rename_vg_action       = new KAction( KIcon("edit-rename"),  i18n("Rename Volume Group..."), this);
    rescan_action          = new KAction( KIcon("view-refresh"), i18n("Rescan System"), this);
    restart_pvmove_action  = new KAction( KIcon("system-restart"), i18n("Restart interrupted pvmove"), this);
    stop_pvmove_action     = new KAction( KIcon("process-stop"),   i18n("Abort pvmove"), this);
    remove_missing_action  = new KAction( KIcon("error_go"),       i18n("Remove Missing Physcial Volumes..."), this);
    export_vg_action       = new KAction( KIcon("document-export"), i18n("Export Volume Group..."), this);
    import_vg_action       = new KAction( KIcon("document-import"), i18n("Import Volume Group..."), this);
    merge_vg_action        = new KAction( KIcon("arrow_join"),   i18n("Merge Volume Group..."), this);
    split_vg_action        = new KAction( KIcon("arrow_divide"), i18n("Split Volume Group..."), this);
    change_vg_action       = new KAction( KIcon("wrench"), i18n("Change Volume Group Attributes..."), this);
    create_vg_action       = new KAction( KIcon("document-new"), i18n("Create Volume Group..."), this);
    config_kvpm_action     = new KAction( KIcon("configure"),    i18n("Configure kvpm..."), this);

    file_menu->addAction(quit_action);
    tool_menu->addAction(rescan_action);
    tool_menu->addSeparator();
    tool_menu->addAction(restart_pvmove_action);
    tool_menu->addAction(stop_pvmove_action);

    groups_menu->addAction(create_vg_action);
    groups_menu->addAction(remove_vg_action);
    groups_menu->addAction(rename_vg_action);
    groups_menu->addSeparator();
    groups_menu->addAction(remove_missing_action);
    groups_menu->addAction(extend_vg_action);
    groups_menu->addAction(reduce_vg_action);
    groups_menu->addAction(split_vg_action);
    groups_menu->addAction(merge_vg_action);
    groups_menu->addSeparator();
    groups_menu->addAction(export_vg_action);
    groups_menu->addAction(import_vg_action);
    groups_menu->addAction(change_vg_action);

    settings_menu->addAction(config_kvpm_action);
    
    g_master_list = NULL;
    m_tab_widget = NULL;

    connect(change_vg_action,  SIGNAL(triggered()), 
	    this, SLOT(changeVolumeGroup()));

    connect(create_vg_action,       SIGNAL(triggered()), 
	    this, SLOT(createVolumeGroup()));

    connect(remove_vg_action,       SIGNAL(triggered()), 
	    this, SLOT(removeVolumeGroup()));

    connect(rename_vg_action,       SIGNAL(triggered()), 
	    this, SLOT(renameVolumeGroup()));

    connect(export_vg_action,       SIGNAL(triggered()), 
	    this, SLOT(exportVolumeGroup()));

    connect(extend_vg_action,       SIGNAL(triggered()), 
	    this, SLOT(extendVolumeGroup()));

    connect(import_vg_action,       SIGNAL(triggered()), 
	    this, SLOT(importVolumeGroup()));

    connect(split_vg_action,       SIGNAL(triggered()), 
	    this, SLOT(splitVolumeGroup()));

    connect(merge_vg_action,       SIGNAL(triggered()), 
	    this, SLOT(mergeVolumeGroup()));

    connect(remove_missing_action,  SIGNAL(triggered()), 
	    this, SLOT(removeMissingVolumes()));

    connect(reduce_vg_action,       SIGNAL(triggered()), 
	    this, SLOT(reduceVolumeGroup()));

    connect(rescan_action,          SIGNAL(triggered()), 
	    this, SLOT(reRun()));

    connect(restart_pvmove_action, SIGNAL(triggered()), 
	    this, SLOT(restartPhysicalVolumeMove()));

    connect(stop_pvmove_action,    SIGNAL(triggered()), 
	    this, SLOT(stopPhysicalVolumeMove()));

    connect(config_kvpm_action,    SIGNAL(triggered()), 
	    this, SLOT(configKvpm()));

    m_tab_widget = new MainTabWidget(this);
    setCentralWidget(m_tab_widget);

    m_device_tab = new DeviceTab();
    m_tab_widget->appendDeviceTab(m_device_tab, i18n("Storage devices") );

    g_master_list = new MasterList(); // creates *empty* masterlist
    menuBar()->setCornerWidget( g_master_list->getProgressBar() );

    connect(qApp, SIGNAL(aboutToQuit()), 
	    this, SLOT(cleanUp()));

    reRun();    // reRun also does the initial run
}

void TopWindow::reRun()
{
    g_master_list->rescan(); // loads the list with data

    updateTrees();
}

void TopWindow::updateTrees()
{
    VolumeGroupTab *tab;
    QList<VolGroup *> groups;
    bool vg_exists;

    disconnect(m_tab_widget, SIGNAL(currentIndexChanged()), 
	    this, SLOT(setupMenus()));

    m_device_tab->rescan( g_master_list->getStorageDevices() );

    groups = g_master_list->getVolGroups();
    // if there is a tab for a deleted vg then delete the tab

    for(int x = 1; x < m_tab_widget->getCount(); x++){
        vg_exists = false;
        for(int y = 0; y < groups.size(); y++){
            if( m_tab_widget->getUnmungedText(x) == groups[y]->getName() )
                vg_exists = true;
        }
        if( !vg_exists ){
            m_tab_widget->deleteTab(x);
        }
    }
    // if there is a new vg and no tab then create one
        
    for( int y = 0; y < groups.size(); y++){
        vg_exists = false;   
        for(int x = 1; x < m_tab_widget->getCount(); x++){
            if( m_tab_widget->getUnmungedText(x) == groups[y]->getName() ){
                vg_exists = true;
                if(groups[y]->isPartial())
                    m_tab_widget->setIcon( x, KIcon("exclamation") );
                else
                    m_tab_widget->setIcon( x, KIcon() );
            }
        }
        if( !vg_exists ){
            tab = new VolumeGroupTab(groups[y]);
            if( groups[y]->isPartial() )
                m_tab_widget->appendVolumeGroupTab( tab, KIcon("exclamation"),groups[y]->getName() );
            else
                m_tab_widget->appendVolumeGroupTab( tab, KIcon(), groups[y]->getName() );
        }
    }

    for(int x = 0; x < (m_tab_widget->getCount() - 1); x++)
        m_tab_widget->getVolumeGroupTab(x)->rescan();

    setupMenus();

    connect(m_tab_widget, SIGNAL(currentIndexChanged()), 
	    this, SLOT(setupMenus()));

}

void TopWindow::setupMenus()
{
    int index = m_tab_widget->getCurrentIndex();
    bool has_active = false;
    QList<LogVol *> lvs;

    if(index){
        m_vg = g_master_list->getVolGroupByName( m_tab_widget->getUnmungedText(index) );
        if( m_vg != NULL ){
            lvs = m_vg->getLogicalVolumes();
            for( int x = lvs.size() - 1; x >= 0 ;x-- ){
                if( lvs[x]->isActive() )
                    has_active = true;
            }
        }
    }
    else 
	m_vg = NULL;

    // only enable group removal if the tab is
    // a volume group with no logical volumes

    if(m_vg){                                     

	if( lvs.size() || m_vg->isPartial() || m_vg->isExported() )             
	    remove_vg_action->setEnabled(false);
	else
	    remove_vg_action->setEnabled(true);   

	if( m_vg->isPartial() )
	    remove_missing_action->setEnabled(true);
        else
	    remove_missing_action->setEnabled(false);

	if( m_vg->isExported() ){
            split_vg_action->setEnabled(false);	
            merge_vg_action->setEnabled(false);	
	    import_vg_action->setEnabled(true);
	    export_vg_action->setEnabled(false);
            reduce_vg_action->setEnabled(false);
            extend_vg_action->setEnabled(false);	
	}
	else if( !m_vg->isPartial() ){
	    import_vg_action->setEnabled(false);
            reduce_vg_action->setEnabled(true);
            split_vg_action->setEnabled(true);	
            merge_vg_action->setEnabled(true);	
            extend_vg_action->setEnabled(true);	

            if(has_active)
                export_vg_action->setEnabled(false);
            else
                export_vg_action->setEnabled(true);
	}
        else{
            split_vg_action->setEnabled(false);	
            merge_vg_action->setEnabled(false);	
	    import_vg_action->setEnabled(false);
	    export_vg_action->setEnabled(false);
            reduce_vg_action->setEnabled(false);
            extend_vg_action->setEnabled(false);	
        }

	rename_vg_action->setEnabled(true);      
	change_vg_action->setEnabled(true);
    }
    else{
	reduce_vg_action->setEnabled(false);
	rename_vg_action->setEnabled(false);      
	remove_vg_action->setEnabled(false);
	remove_missing_action->setEnabled(false);
	change_vg_action->setEnabled(false);
	import_vg_action->setEnabled(false);
	split_vg_action->setEnabled(false);
	merge_vg_action->setEnabled(false);
	export_vg_action->setEnabled(false);
	extend_vg_action->setEnabled(false);
    }
}

void TopWindow::changeVolumeGroup()
{
    if( change_vg(m_vg) )
	reRun();
}

void TopWindow::createVolumeGroup()
{
    if( create_vg() )
        reRun();
}

void TopWindow::removeVolumeGroup()
{
    if( remove_vg(m_vg) )
        reRun();
}

void TopWindow::renameVolumeGroup()
{
    if( rename_vg(m_vg) )
        reRun();
}

void TopWindow::reduceVolumeGroup()
{
    if( reduce_vg(m_vg) )
        reRun();
}

void TopWindow::exportVolumeGroup()
{
    if( export_vg(m_vg) )
        reRun();
}

void TopWindow::extendVolumeGroup()
{
    if( extend_vg(m_vg) )
        reRun();
}

void TopWindow::importVolumeGroup()
{
    if( import_vg(m_vg) )
        reRun();
}

void TopWindow::splitVolumeGroup()
{
    if( split_vg(m_vg) )
        reRun();
}

void TopWindow::mergeVolumeGroup()
{
    if( merge_vg(m_vg) )
        reRun();
}

void TopWindow::removeMissingVolumes()
{
    if( remove_missing_pv(m_vg) )
        reRun();
}

void TopWindow::restartPhysicalVolumeMove()
{
    if( restart_pvmove() )
        reRun();
}

void TopWindow::stopPhysicalVolumeMove()
{
    if( stop_pvmove() )
        reRun();
}

void TopWindow::configKvpm()
{
    KvpmConfigDialog *const dialog  = new KvpmConfigDialog( this, "settings", new KConfigSkeleton() );

    connect(dialog, SIGNAL(applyClicked()), 
	    this,   SLOT(updateTrees()));

    dialog->exec();

    disconnect(dialog, SIGNAL(applyClicked()), 
               this,   SLOT(updateTrees()));

    dialog->deleteLater();
    updateTrees();
}

void TopWindow::cleanUp()
{
    delete g_master_list;  // This calls lvm_quit() on destruct
}

void TopWindow::closeEvent(QCloseEvent *)
{
    qApp->quit();
}
