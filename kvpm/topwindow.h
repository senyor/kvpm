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

#ifndef TOPWINDOW_H
#define TOPWINDOW_H

#include <KMainWindow>
#include <KTabWidget>
#include <KMenu>
#include <KMenuBar>
#include <KAction>
#include <QStringList>
#include <QVBoxLayout>

class StorageDevice;
class VolGroup;
class VolumeGroupTab;
class DeviceTab;

class TopWindow : public KMainWindow
{
Q_OBJECT
    KTabWidget *tab_widget, *old_tab_widget;

    KAction *quit_action, *remove_vg_action, *reduce_vg_action, *rescan_action, 
	    *rescan_vg_action, *vgchange_alloc_action, *vgchange_pv_action, 
	    *vgchange_lv_action, *vgchange_resize_action,
	    *vgchange_extent_action, *remove_missing_action, *config_kvpm_action;

    KMenu *vgchange_menu, *file_menu, *tool_menu, 
	  *groups_menu, *settings_menu,
	  *help_menu;
    
    QVBoxLayout *layout;
    QList<VolumeGroupTab *> vg_tabs;
    DeviceTab *device_tab;
    VolGroup *vg;

 public:
    TopWindow(QWidget *parent);
    
 public slots:
    void reRun();
    void rebuildVolumeGroupTab();
 
 private slots:
    void setupMenus(int index);
    void updateTabGeometry(int index);
    void launchVGChangeAllocDialog();
    void launchVGChangeExtentDialog();
    void launchVGChangeLVDialog();
    void launchVGChangePVDialog();
    void launchVGChangeResizeDialog();
    void launchVGRemoveDialog();
    void launchRemoveMissingDialog();
    void launchVGReduceDialog();
};

extern TopWindow *MainWindow;

#endif
