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

#ifndef TOPWINDOW_H
#define TOPWINDOW_H


#include <KMainWindow>

class KAction;
class KMenu;
class KMenuBar;
class KToggleAction;

class QVBoxLayout;

class DeviceTab;
class ExecutableFinder;
class MainTabWidget;
class MasterList;
class ProgressBox;
class StorageDevice;
class TopWindow;
class VolGroup;
class VolumeGroupTab;

extern TopWindow *MainWindow;


class TopWindow : public KMainWindow
{
    Q_OBJECT
    MainTabWidget *m_tab_widget = nullptr;          // The current tab widget we are using

    KAction *m_remove_action, *m_rename_action, *m_extend_action,
            *m_reduce_action, *m_create_action, *m_merge_action,
            *m_change_action, *m_remove_missing_action,
            *m_export_action, *m_import_action, *m_split_action;

    DeviceTab  *m_device_tab = nullptr;
    VolGroup   *m_vg = nullptr;
    MasterList *m_master_list;
    ExecutableFinder *m_executable_finder;

    static ProgressBox *m_progress_box;

    void closeEvent(QCloseEvent *);
    KMenu *buildFileMenu();
    KMenu *buildGroupsMenu();
    KMenu *buildHelpMenu();
    KMenu *buildSettingsMenu();
    KMenu *buildToolbarSizeMenu();
    KMenu *buildToolbarTextMenu();
    KMenu *buildToolsMenu();

public:
    TopWindow(MasterList *const masterList, ExecutableFinder *const executableFinder, QWidget *parent = nullptr);
    static ProgressBox *getProgressBox();
    void reRun();

public slots:
    void updateTabs();

private slots:
    void cleanUp();
    void setupMenus();
    void showVolumeGroupInfo(bool show);
    void showVolumeGroupBar(bool show);
    void showToolbars(bool show);
    void useSiUnits(bool use);
    void setToolbarIconSize(QAction *action);
    void setToolbarIconText(QAction *action);
    void callVgDialog(QAction *action);
    void callToolsDialog(QAction *action);
    void configKvpm();

};

#endif
