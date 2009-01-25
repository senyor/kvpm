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
class TopWindow;
class VolGroup;
class VolumeGroupTab;
class DeviceTab;

extern TopWindow *MainWindow;

class TopWindow : public KMainWindow
{
Q_OBJECT
    KTabWidget *m_tab_widget,          // The current tab widget we are using 
               *m_old_tab_widget;      // The tab widget we have schedualed for deletion 

    KAction *quit_action, *remove_vg_action, *rename_vg_action, 
            *reduce_vg_action, *rescan_action, 
	    *rescan_vg_action, *vgchange_alloc_action, *vgchange_pv_action, 
	    *vgchange_lv_action, *vgchange_resize_action, *vgchange_available_action, 
	    *vgchange_extent_action, *remove_missing_action, *config_kvpm_action;

    KAction *m_restart_pvmove_action,
	    *m_stop_pvmove_action,
	    *m_export_vg_action,
	    *m_import_vg_action;
    
    KMenu *m_vgchange_menu;

    QList<VolumeGroupTab *> m_vg_tabs;
    QList<VolumeGroupTab *> m_old_vg_tabs;  // These widgets are schedualed for deletion 

    DeviceTab *m_device_tab;
    VolGroup *m_vg;

 public:
    TopWindow(QWidget *parent);
    
 public slots:
    void reRun();
    void rebuildVolumeGroupTab();
 
 private slots:
    void setupMenus(int index);
    void updateTabGeometry(int index);
    void changeAllocation();
    void changeAvailable();
    void changeExtentSize();
    void limitLogicalVolumes();
    void limitPhysicalVolumes();
    void changeResize();
    void removeVolumeGroup();
    void renameVolumeGroup();
    void removeMissingVolumes();
    void reduceVolumeGroup();
    void exportVolumeGroup();
    void importVolumeGroup();
    void restartPhysicalVolumeMove();
    void stopPhysicalVolumeMove();
    void configKvpm();
    
};

#endif
