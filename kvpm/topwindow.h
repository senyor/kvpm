/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2016 Benjamin Scott   <benscott@nwlink.com>
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


class QMenu;

class DeviceTab;
class ExecutableFinder;
class MainTabWidget;
class MasterList;
class ProgressBox;
class TopWindow;
class VGActions;

extern TopWindow *g_top_window;


class TopWindow : public KMainWindow
{
    Q_OBJECT

    MainTabWidget *m_tab_widget = nullptr;          // The current tab widget we are using
    VGActions *m_vg_actions = nullptr;
    DeviceTab *m_device_tab = nullptr;

    MasterList *m_master_list;
    ExecutableFinder *m_executable_finder;

    static ProgressBox *m_progress_box;

    void closeEvent(QCloseEvent *);
    QMenu *buildFileMenu();
    QMenu *buildGroupsMenu();
    QMenu *buildHelpMenu();
    QMenu *buildSettingsMenu();
    QMenu *buildToolbarSizeMenu();
    QMenu *buildToolbarTextMenu();
    QMenu *buildToolsMenu();

public:
    TopWindow(MasterList *const masterList, ExecutableFinder *const executableFinder, QWidget *parent = nullptr);
    static ProgressBox *getProgressBox();
    void reRun();

public slots:
    void updateTabs();

private slots:
    void cleanUp();
    void setVgMenu(int index);
    void showVolumeGroupInfo(bool show);
    void showVolumeGroupBar(bool show);
    void showDeviceBar(bool show);
    void showToolbars(bool show);
    void useSiUnits(bool use);
    void setToolbarIconSize(QAction *action);
    void setToolbarIconText(QAction *action);
    void callToolsDialog(QAction *action);
    void configKvpm();

};

#endif
