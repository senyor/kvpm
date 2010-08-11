/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
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
#include <KMenu>
#include <KMenuBar>
#include <KAction>

#include <QVBoxLayout>

class StorageDevice;
class TopWindow;
class VolGroup;
class VolumeGroupTab;
class DeviceTab;
class MainTabWidget;

extern TopWindow *MainWindow;

class TopWindow : public KMainWindow
{
Q_OBJECT
    MainTabWidget *m_tab_widget;          // The current tab widget we are using 

    KAction *quit_action,            *remove_vg_action,       *rename_vg_action, 
            *reduce_vg_action,       *rescan_action,          *create_vg_action,
	    *vgchange_alloc_action,  *vgchange_pv_action,     
	    *vgchange_lv_action,     *vgchange_resize_action, *vgchange_available_action, 
	    *vgchange_extent_action, *remove_missing_action,  *config_kvpm_action;

    KAction *restart_pvmove_action, *stop_pvmove_action,
	    *export_vg_action,      *import_vg_action,
            *split_vg_action,       *merge_vg_action;
    
    KMenu *m_vgchange_menu;

    DeviceTab *m_device_tab;
    VolGroup  *m_vg;

 public:
    TopWindow(QWidget *parent);
    
 public slots:
    void reRun();
 
 private slots:
    void setupMenus();
    void changeAllocation();
    void changeAvailable();
    void changeExtentSize();
    void limitLogicalVolumes();
    void limitPhysicalVolumes();
    void changeResize();
    void createVolumeGroup();
    void removeVolumeGroup();
    void renameVolumeGroup();
    void removeMissingVolumes();
    void reduceVolumeGroup();
    void exportVolumeGroup();
    void importVolumeGroup();
    void splitVolumeGroup();
    void mergeVolumeGroup();
    void restartPhysicalVolumeMove();
    void stopPhysicalVolumeMove();
    void configKvpm();
    
};

#endif
