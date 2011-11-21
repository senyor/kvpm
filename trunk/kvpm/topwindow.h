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

#ifndef TOPWINDOW_H
#define TOPWINDOW_H

#include <KAction>
#include <KMainWindow>
#include <KMenu>
#include <KMenuBar>
#include <KToggleAction>

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

    KAction *m_remove_vg_action,      *m_rename_vg_action, 
            *m_reduce_vg_action,      *m_create_vg_action,      *m_extend_vg_action,
            *m_change_vg_action,      *m_remove_missing_action, *m_merge_vg_action,
            *m_export_vg_action,      *m_import_vg_action,      *m_split_vg_action;
            
    DeviceTab *m_device_tab;
    VolGroup  *m_vg;

    void closeEvent(QCloseEvent *);
    KMenu *buildFileMenu();
    KMenu *buildGroupsMenu();
    KMenu *buildHelpMenu();
    KMenu *buildSettingsMenu();
    KMenu *buildToolsMenu();

 public:
    TopWindow(QWidget *parent);
    
 public slots:
    void reRun();
    void updateTabs();
 
 private slots:
    void cleanUp();
    void setupMenus();
    void showVolumeGroupInfo(bool show);
    void showVolumeGroupBar(bool show);
    void changeVolumeGroup();
    void createVolumeGroup();
    void removeVolumeGroup();
    void renameVolumeGroup();
    void removeMissingVolumes();
    void reduceVolumeGroup();
    void exportVolumeGroup();
    void importVolumeGroup();
    void splitVolumeGroup();
    void mergeVolumeGroup();
    void extendVolumeGroup();
    void restartPhysicalVolumeMove();
    void stopPhysicalVolumeMove();
    void configKvpm();
    
};

#endif
