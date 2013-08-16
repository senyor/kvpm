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


#include "topwindow.h"

#include <KAboutData>
#include <KActionCollection>
#include <KAction>
#include <KApplication>
#include <KConfigSkeleton>
#include <KHelpMenu>
#include <KIcon>
#include <KLocale>
#include <KMenu>
#include <KMenuBar>
#include <KStandardAction>
#include <KToggleAction>

#include <QActionGroup>
#include <QDebug>
#include <QVBoxLayout>

#include "kvpmconfigdialog.h"
#include "devicetab.h"
#include "executablefinder.h"
#include "masterlist.h"
#include "maintabwidget.h"
#include "processprogress.h"
#include "progressbox.h"
#include "pvmove.h"
#include "vgactions.h"
#include "volgroup.h"
#include "volumegrouptab.h"


ProgressBox* TopWindow::m_progress_box = nullptr;   // Static initializing


TopWindow::TopWindow(MasterList *const masterList, ExecutableFinder *const executableFinder, QWidget *parent) : 
    KMainWindow(parent),
    m_master_list(masterList),
    m_executable_finder(executableFinder)
{
    m_progress_box = new ProgressBox(); // make sure this stays *before* master_list->rescan() gets called!

    menuBar()->addMenu(buildFileMenu());
    menuBar()->addMenu(buildGroupsMenu());
    menuBar()->addMenu(buildToolsMenu());
    menuBar()->addMenu(buildSettingsMenu());
    menuBar()->addMenu(buildHelpMenu());

    m_tab_widget = new MainTabWidget(this);
    setCentralWidget(m_tab_widget);

    m_device_tab = new DeviceTab();
    m_tab_widget->appendDeviceTab(m_device_tab, i18n("Storage Devices"));

    menuBar()->setCornerWidget(m_progress_box);

    connect(qApp, SIGNAL(aboutToQuit()),
            this, SLOT(cleanUp()));

    reRun();    // reRun also does the initial run
}

void TopWindow::reRun()
{
    qApp->setOverrideCursor(Qt::WaitCursor);
    m_master_list->rescan(); // loads the list with new data
    updateTabs();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    qApp->restoreOverrideCursor();
}

ProgressBox* TopWindow::getProgressBox()
{
    return m_progress_box;
}

void TopWindow::updateTabs()
{
    VolumeGroupTab *tab = nullptr;
    bool vg_exists = false;

    disconnect(m_tab_widget, SIGNAL(currentIndexChanged(int)),
               this, SLOT(setVgMenu(int)));

    m_device_tab->rescan(MasterList::getStorageDevices());

    QList<VolGroup *> groups = MasterList::getVolGroups();
    // if there is a tab for a deleted vg then delete the tab

    for (int x = 1; x < m_tab_widget->getCount(); ++x) {
        vg_exists = false;

        for (auto vg : groups) {
            if (m_tab_widget->getUnmungedText(x) == vg->getName())
                vg_exists = true;
        }

        if (!vg_exists)
            m_tab_widget->deleteTab(x);
    }
    // if there is a new vg and no tab then create one

    for (int y = 0; y < groups.size(); ++y) {
        vg_exists = false;
        for (int x = 1; x < m_tab_widget->getCount(); ++x) {
            if (m_tab_widget->getUnmungedText(x) == groups[y]->getName()) {
                vg_exists = true;
                if (groups[y]->isPartial())
                    m_tab_widget->setIcon(x, KIcon("exclamation"));
                else
                    m_tab_widget->setIcon(x, KIcon());
            }
        }

        if (!vg_exists) {
            tab = new VolumeGroupTab(groups[y]);
            if (groups[y]->isPartial())
                m_tab_widget->appendVolumeGroupTab(tab, KIcon("exclamation"), groups[y]->getName());
            else
                m_tab_widget->appendVolumeGroupTab(tab, KIcon(), groups[y]->getName());
        }
    }

    for (int x = 0; x < (m_tab_widget->getCount() - 1); ++x)
        m_tab_widget->getVolumeGroupTab(x)->rescan();

    setVgMenu(m_tab_widget->getCurrentIndex());

    connect(m_tab_widget, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setVgMenu(int)));
}

void TopWindow::callToolsDialog(QAction *action)
{
    if (action->objectName() == "restartpvmove") {
        if (restart_pvmove())
            reRun();
    } else if (action->objectName() == "stoppvmove") {
        if (stop_pvmove())
            reRun();
    } else if (action->objectName() == "systemreload") {
        reRun();
    }
}

void TopWindow::configKvpm()
{
    KvpmConfigDialog *const dialog = new KvpmConfigDialog(this, "settings", new KConfigSkeleton(), m_executable_finder);

    connect(dialog, SIGNAL(applyClicked()),
            this,   SLOT(updateTabs()));

    dialog->exec();

    disconnect(dialog, SIGNAL(applyClicked()),
               this,   SLOT(updateTabs()));

    dialog->deleteLater();
    updateTabs();
}

void TopWindow::cleanUp()
{
    delete m_master_list;  // This calls lvm_quit() on destruct
}

void TopWindow::closeEvent(QCloseEvent *)
{
    qApp->quit();
}

void TopWindow::showVolumeGroupInfo(bool show)
{
    KConfigSkeleton skeleton;
    bool show_vg_info;

    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("show_vg_info", show_vg_info, true);
    show_vg_info = show;
    skeleton.writeConfig();
    updateTabs();
}

void TopWindow::showVolumeGroupBar(bool show)
{
    KConfigSkeleton skeleton;
    bool show_lv_bar;

    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("show_lv_bar", show_lv_bar, true);
    show_lv_bar = show;
    skeleton.writeConfig();
    updateTabs();
}

void TopWindow::showDeviceBar(bool show)
{
    KConfigSkeleton skeleton;
    bool show_device_bar;
    qDebug() << "Got here ....";
    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("show_device_barchart", show_device_bar, true);
    show_device_bar = show;
    skeleton.writeConfig();
    updateTabs();
}

void TopWindow::showToolbars(bool show)
{
    KConfigSkeleton skeleton;
    bool show_toolbars;

    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("show_toolbars", show_toolbars, true);
    show_toolbars = show;
    skeleton.writeConfig();
    updateTabs();
}

void TopWindow::useSiUnits(bool use)
{
    KConfigSkeleton skeleton;
    bool use_si_units;

    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", use_si_units, false);
    use_si_units = use;
    skeleton.writeConfig();
    updateTabs();
}

void TopWindow::setToolbarIconSize(QAction *action)
{
    KConfigSkeleton skeleton;
    QString icon_size;

    skeleton.setCurrentGroup("General");
    skeleton.addItemString("toolbar_icon_size", icon_size, "mediumicons");
    icon_size = action->objectName();
    skeleton.writeConfig();
    updateTabs();
}

void TopWindow::setToolbarIconText(QAction *action)
{
    KConfigSkeleton skeleton;
    QString icon_text;

    skeleton.setCurrentGroup("General");
    skeleton.addItemString("toolbar_icon_text", icon_text, "iconsonly");
    icon_text = action->objectName();
    skeleton.writeConfig();
    updateTabs();
}

KMenu *TopWindow::buildToolbarSizeMenu()
{
    KMenu *const menu = new KMenu(i18n("Toolbar icon size"));

    KToggleAction *const small_action = new KToggleAction(i18n("Small icons (16x16)"), this);
    KToggleAction *const medium_action = new KToggleAction(i18n("Medium icons (22x22)"), this);
    KToggleAction *const large_action = new KToggleAction(i18n("Large icons (32x32)"), this);
    KToggleAction *const huge_action = new KToggleAction(i18n("Huge icons (48x48)"), this);

    small_action->setObjectName("smallicons");
    medium_action->setObjectName("mediumicons");
    large_action->setObjectName("largeicons");
    huge_action->setObjectName("hugeicons");

    QActionGroup  *const size_group = new QActionGroup(this);

    size_group->addAction(small_action);
    size_group->addAction(medium_action);
    size_group->addAction(large_action);
    size_group->addAction(huge_action);

    connect(size_group, SIGNAL(triggered(QAction *)),
            this, SLOT(setToolbarIconSize(QAction *)));

    menu->addAction(small_action);
    menu->addAction(medium_action);
    menu->addAction(large_action);
    menu->addAction(huge_action);

    KConfigSkeleton skeleton;
    QString icon_size;

    skeleton.setCurrentGroup("General");
    skeleton.addItemString("toolbar_icon_size", icon_size, "mediumicons");
    
    if (icon_size == "smallicons")
        small_action->setChecked(true);
    else if (icon_size == "mediumicons")
        medium_action->setChecked(true);
    else if (icon_size == "largeicons")
        large_action->setChecked(true);
    else if (icon_size == "hugeicons")
        huge_action->setChecked(true);

    return menu;
}

KMenu *TopWindow::buildToolbarTextMenu()
{
    KMenu *const menu = new KMenu(i18n("Toolbar text placement"));

    KToggleAction *const icons_only_action = new KToggleAction(i18n("Icons only"), this);
    KToggleAction *const text_only_action = new KToggleAction(i18n("Text only"), this);
    KToggleAction *const text_beside_action = new KToggleAction(i18n("Text beside icons"), this);
    KToggleAction *const text_under_action = new KToggleAction(i18n("Text under icons"), this);

    QActionGroup  *const icon_group = new QActionGroup(this);
    
    icons_only_action->setObjectName("iconsonly");
    text_only_action->setObjectName("textonly");
    text_beside_action->setObjectName("textbesideicons");
    text_under_action->setObjectName("textundericons");

    icon_group->addAction(icons_only_action);
    icon_group->addAction(text_only_action);
    icon_group->addAction(text_beside_action);
    icon_group->addAction(text_under_action);

    connect(icon_group, SIGNAL(triggered(QAction *)),
            this, SLOT(setToolbarIconText(QAction *)));

    menu->addAction(icons_only_action);
    menu->addAction(text_only_action);
    menu->addAction(text_beside_action);
    menu->addAction(text_under_action);

    KConfigSkeleton skeleton;
    QString icon_text;

    skeleton.setCurrentGroup("General");
    skeleton.addItemString("toolbar_icon_text", icon_text, "iconsonly");
    
    if (icon_text == "iconsonly")
        icons_only_action->setChecked(true);
    else if (icon_text == "textonly")
        text_only_action->setChecked(true);
    else if (icon_text == "textbesideicons")
        text_beside_action->setChecked(true);
    else if (icon_text == "textundericons")
        text_under_action->setChecked(true);

    return menu;
}

KMenu *TopWindow::buildSettingsMenu()
{
    KMenu *const menu = new KMenu(i18n("Settings"));
    KToggleAction *const show_vg_info_action  = new KToggleAction(i18n("Show Volume Group Information"), this);
    KToggleAction *const show_lv_bar_action   = new KToggleAction(i18n("Show Volume Group Bar Chart"), this);
    KToggleAction *const show_device_bar_action   = new KToggleAction(i18n("Show Device Bar Chart"), this);
    KToggleAction *const use_si_units_action  = new KToggleAction(i18n("Use Metric SI Units"), this);
    KToggleAction *const show_toolbars_action = new KToggleAction(i18n("Show Toolbars"), this);
    KAction *const config_kvpm_action = new KAction(KIcon("configure"), i18n("Configure kvpm..."), this);

    menu->addAction(show_vg_info_action);
    menu->addAction(show_lv_bar_action);
    menu->addAction(show_device_bar_action);
    menu->addAction(use_si_units_action);
    menu->addSeparator();
    menu->addAction(show_toolbars_action);
    menu->addMenu(buildToolbarTextMenu());
    menu->addMenu(buildToolbarSizeMenu());
    menu->addSeparator();
    menu->addAction(config_kvpm_action);

    KConfigSkeleton skeleton;
    bool show_vg_info, show_lv_bar, use_si_units, show_toolbars, show_device_bar;

    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("show_vg_info", show_vg_info, true);
    skeleton.addItemBool("show_lv_bar",  show_lv_bar,  true);
    skeleton.addItemBool("show_device_barchart", show_device_bar,  true);
    skeleton.addItemBool("use_si_units", use_si_units, false);
    skeleton.addItemBool("show_toolbars", show_toolbars, true);

    // This must be *before* the following connect() statements
    show_vg_info_action->setChecked(show_vg_info);
    show_lv_bar_action->setChecked(show_lv_bar);
    show_device_bar_action->setChecked(show_device_bar);
    use_si_units_action->setChecked(use_si_units);
    show_toolbars_action->setChecked(show_toolbars);

    connect(show_vg_info_action, SIGNAL(toggled(bool)),
            this, SLOT(showVolumeGroupInfo(bool)));

    connect(show_lv_bar_action,  SIGNAL(toggled(bool)),
            this, SLOT(showVolumeGroupBar(bool)));

    connect(show_device_bar_action,  SIGNAL(toggled(bool)),
            this, SLOT(showDeviceBar(bool)));

    connect(use_si_units_action, SIGNAL(toggled(bool)),
            this, SLOT(useSiUnits(bool)));

    connect(show_toolbars_action, SIGNAL(toggled(bool)),
            this, SLOT(showToolbars(bool)));

    connect(config_kvpm_action,  SIGNAL(triggered()),
            this, SLOT(configKvpm()));

    return menu;
}

KMenu *TopWindow::buildToolsMenu()
{
    KMenu   *const menu = new KMenu(i18n("Tools"));
    KAction *const rescan_action         = new KAction(KIcon("view-refresh"),  i18n("Rescan System"), this);
    KAction *const restart_pvmove_action = new KAction(KIcon("system-reboot"), i18n("Restart interrupted pvmove"), this);
    KAction *const stop_pvmove_action    = new KAction(KIcon("process-stop"),  i18n("Abort pvmove"), this);

    rescan_action->setObjectName("systemreload");
    restart_pvmove_action->setObjectName("restartpvmove");
    stop_pvmove_action->setObjectName("stoppvmove");

    menu->addAction(rescan_action);
    menu->addSeparator();
    menu->addAction(restart_pvmove_action);
    menu->addAction(stop_pvmove_action);

    QActionGroup *const actions = new QActionGroup(this);

    actions->addAction(rescan_action);
    actions->addAction(restart_pvmove_action);
    actions->addAction(stop_pvmove_action);

    connect(actions, SIGNAL(triggered(QAction *)),
            this, SLOT(callToolsDialog(QAction *)));

    return menu;
}

KMenu *TopWindow::buildHelpMenu()
{
    KAboutData *const about_data = new KAboutData(QByteArray("kvpm"),
            QByteArray("kvpm"),
            ki18n("kvpm"),
            QByteArray("0.9.6"),
            ki18n("Linux volume and partition manager for KDE.  "
                  "This program is still under development, "
                  "bug reports and any comments are welcomed.  "
                  "Licensed under GNU General Public License v3.0"),
            KAboutData::License_GPL_V3,
            ki18n("(c) 2008, 2009, 2010, 2011, 2012, 2013 Benjamin Scott"),
            KLocalizedString(),
            QByteArray("http://sourceforge.net/projects/kvpm/"),
            QByteArray("benscott@nwlink.com"));

    about_data->addCredit(ki18n("Mark James"),
                          ki18n("Additional icons taken from the Silk icon set by "
                                "Mark James under the Creative Commons Attribution 3.0 License."),
                          QByteArray(),
                          QByteArray("http://www.famfamfam.com/lab/icons/silk/"));

    KHelpMenu *const help_menu = new KHelpMenu(this, about_data);

    return help_menu->menu();
}

KMenu *TopWindow::buildFileMenu()
{
    KMenu *const menu = new KMenu(i18n("File"));
    KAction *const quit_action = KStandardAction::quit(qApp, SLOT(quit()), menu);
    menu->addAction(quit_action);

    return menu;
}

void TopWindow::setVgMenu(int index)
{
    m_vg_actions->setVg(MasterList::getVgByName(m_tab_widget->getUnmungedText(index)));
}

KMenu *TopWindow::buildGroupsMenu()
{
    m_vg_actions = new VGActions(this);

    KMenu *const menu = new KMenu(i18n("Volume Groups"));

    menu->addAction(m_vg_actions->action("vgcreate"));
    menu->addAction(m_vg_actions->action("vgremove"));
    menu->addAction(m_vg_actions->action("vgrename"));
    menu->addSeparator();
    menu->addAction(m_vg_actions->action("vgremovemissing"));
    menu->addAction(m_vg_actions->action("vgextend"));
    menu->addAction(m_vg_actions->action("vgreduce"));
    menu->addAction(m_vg_actions->action("vgsplit"));
    menu->addAction(m_vg_actions->action("vgmerge"));
    menu->addSeparator();
    menu->addAction(m_vg_actions->action("vgimport"));
    menu->addAction(m_vg_actions->action("vgexport"));
    menu->addAction(m_vg_actions->action("vgchange"));

    return menu;
}


