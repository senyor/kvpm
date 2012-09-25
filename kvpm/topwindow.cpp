/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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
#include <KApplication>
#include <KAction>
#include <KConfigSkeleton>
#include <KHelpMenu>
#include <KIcon>
#include <KLocale>
#include <KMenu>
#include <KMenuBar>
#include <KStandardAction>
#include <KToggleAction>

#include <QVBoxLayout>

#include "kvpmconfigdialog.h"
#include "devicetab.h"
#include "executablefinder.h"
#include "logvol.h"
#include "masterlist.h"
#include "maintabwidget.h"
#include "physvol.h"
#include "processprogress.h"
#include "progressbox.h"
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


ProgressBox* TopWindow::m_progress_box = NULL;   // Static initializing


TopWindow::TopWindow(MasterList *const masterList, ExecutableFinder *const executableFinder, QWidget *parent)
    : KMainWindow(parent),
      m_master_list(masterList),
      m_executable_finder(executableFinder)
{
    m_tab_widget = NULL;
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
    VolumeGroupTab *tab;
    QList<VolGroup *> groups;
    bool vg_exists;

    disconnect(m_tab_widget, SIGNAL(currentIndexChanged()),
               this, SLOT(setupMenus()));

    m_device_tab->rescan(MasterList::getStorageDevices());

    groups = MasterList::getVolGroups();
    // if there is a tab for a deleted vg then delete the tab

    for (int x = 1; x < m_tab_widget->getCount(); x++) {
        vg_exists = false;
        for (int y = 0; y < groups.size(); y++) {
            if (m_tab_widget->getUnmungedText(x) == groups[y]->getName())
                vg_exists = true;
        }
        if (!vg_exists) {
            m_tab_widget->deleteTab(x);
        }
    }
    // if there is a new vg and no tab then create one

    for (int y = 0; y < groups.size(); y++) {
        vg_exists = false;
        for (int x = 1; x < m_tab_widget->getCount(); x++) {
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

    for (int x = 0; x < (m_tab_widget->getCount() - 1); x++)
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

    if (index) {
        m_vg = MasterList::getVgByName(m_tab_widget->getUnmungedText(index));
        if (m_vg != NULL) {
            lvs = m_vg->getLogicalVolumes();
            for (int x = lvs.size() - 1; x >= 0 ; x--) {
                if (lvs[x]->isActive())
                    has_active = true;
            }
        }
    } else
        m_vg = NULL;

    // only enable group removal if the tab is
    // a volume group with no logical volumes

    if (m_vg) {

        if (lvs.size() || m_vg->isPartial() || m_vg->isExported())
            m_remove_vg_action->setEnabled(false);
        else
            m_remove_vg_action->setEnabled(true);

        if (m_vg->isPartial())
            m_remove_missing_action->setEnabled(true);
        else
            m_remove_missing_action->setEnabled(false);

        if (m_vg->isExported()) {
            m_split_vg_action->setEnabled(false);
            m_merge_vg_action->setEnabled(false);
            m_import_vg_action->setEnabled(true);
            m_export_vg_action->setEnabled(false);
            m_reduce_vg_action->setEnabled(false);
            m_extend_vg_action->setEnabled(false);
        } else if (!m_vg->isPartial()) {
            m_import_vg_action->setEnabled(false);
            m_reduce_vg_action->setEnabled(true);
            m_split_vg_action->setEnabled(true);
            m_merge_vg_action->setEnabled(true);
            m_extend_vg_action->setEnabled(true);

            if (has_active)
                m_export_vg_action->setEnabled(false);
            else
                m_export_vg_action->setEnabled(true);
        } else {
            m_split_vg_action->setEnabled(false);
            m_merge_vg_action->setEnabled(false);
            m_import_vg_action->setEnabled(false);
            m_export_vg_action->setEnabled(false);
            m_reduce_vg_action->setEnabled(false);
            m_extend_vg_action->setEnabled(false);
        }

        m_rename_vg_action->setEnabled(true);
        m_change_vg_action->setEnabled(true);
    } else {
        m_reduce_vg_action->setEnabled(false);
        m_rename_vg_action->setEnabled(false);
        m_remove_vg_action->setEnabled(false);
        m_remove_missing_action->setEnabled(false);
        m_change_vg_action->setEnabled(false);
        m_import_vg_action->setEnabled(false);
        m_split_vg_action->setEnabled(false);
        m_merge_vg_action->setEnabled(false);
        m_export_vg_action->setEnabled(false);
        m_extend_vg_action->setEnabled(false);
    }
}

void TopWindow::changeVolumeGroup()
{
    VGChangeDialog dialog(m_vg);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            reRun();
    }
}

void TopWindow::createVolumeGroup()
{
    VGCreateDialog dialog;

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            reRun();
    }
}

void TopWindow::removeVolumeGroup()
{
    if (remove_vg(m_vg))
        reRun();
}

void TopWindow::renameVolumeGroup()
{
    if (rename_vg(m_vg))
        reRun();
}

void TopWindow::reduceVolumeGroup()
{
    VGReduceDialog dialog(m_vg);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            reRun();
    }
}

void TopWindow::exportVolumeGroup()
{
    if (export_vg(m_vg))
        reRun();
}

void TopWindow::extendVolumeGroup()
{
    VGExtendDialog dialog(m_vg);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            reRun();
    }
}

void TopWindow::importVolumeGroup()
{
    if (import_vg(m_vg))
        reRun();
}

void TopWindow::splitVolumeGroup()
{
    VGSplitDialog dialog(m_vg);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            reRun();
    }
}

void TopWindow::mergeVolumeGroup()
{
    VGMergeDialog dialog(m_vg);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            reRun();
    }
}

void TopWindow::removeMissingVolumes()
{
    if (remove_missing_pv(m_vg))
        reRun();
}

void TopWindow::restartPhysicalVolumeMove()
{
    if (restart_pvmove())
        reRun();
}

void TopWindow::stopPhysicalVolumeMove()
{
    if (stop_pvmove())
        reRun();
}

void TopWindow::configKvpm()
{
    KvpmConfigDialog *const dialog  = new KvpmConfigDialog(this, "settings", new KConfigSkeleton(), m_executable_finder);

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

KMenu *TopWindow::buildSettingsMenu()
{
    KMenu *const settings_menu = new KMenu(i18n("Settings"));
    KToggleAction *const show_vg_info_action = new KToggleAction(KIcon("preferences-other"), i18n("Show Volume Group Information"), this);
    KToggleAction *const show_lv_bar_action  = new KToggleAction(KIcon("preferences-other"), i18n("Show Volume Group Bar Graph"), this);
    KToggleAction *const use_si_units_action  = new KToggleAction(KIcon("preferences-other"), i18n("Use Metric SI Units"), this);
    KAction *const config_kvpm_action        = new KAction(KIcon("configure"), i18n("Configure kvpm..."), this);

    settings_menu->addAction(show_vg_info_action);
    settings_menu->addAction(show_lv_bar_action);
    settings_menu->addAction(use_si_units_action);
    settings_menu->addSeparator();
    settings_menu->addAction(config_kvpm_action);

    KConfigSkeleton skeleton;
    bool show_vg_info, show_lv_bar, use_si_units;

    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("show_vg_info", show_vg_info, true);
    skeleton.addItemBool("show_lv_bar",  show_lv_bar,  true);
    skeleton.addItemBool("use_si_units", use_si_units, false);

    // This must be *before* the following connect() statements
    show_vg_info_action->setChecked(show_vg_info);
    show_lv_bar_action->setChecked(show_lv_bar);
    use_si_units_action->setChecked(use_si_units);

    connect(config_kvpm_action,  SIGNAL(triggered()),
            this, SLOT(configKvpm()));

    connect(show_vg_info_action, SIGNAL(toggled(bool)),
            this, SLOT(showVolumeGroupInfo(bool)));

    connect(show_lv_bar_action,  SIGNAL(toggled(bool)),
            this, SLOT(showVolumeGroupBar(bool)));

    connect(use_si_units_action, SIGNAL(toggled(bool)),
            this, SLOT(useSiUnits(bool)));

    return settings_menu;
}

KMenu *TopWindow::buildToolsMenu()
{
    KMenu   *const tools_menu = new KMenu(i18n("Tools"));
    KAction *const rescan_action         = new KAction(KIcon("view-refresh"),  i18n("Rescan System"), this);
    KAction *const restart_pvmove_action = new KAction(KIcon("system-reboot"), i18n("Restart interrupted pvmove"), this);
    KAction *const stop_pvmove_action    = new KAction(KIcon("process-stop"),  i18n("Abort pvmove"), this);

    tools_menu->addAction(rescan_action);
    tools_menu->addSeparator();
    tools_menu->addAction(restart_pvmove_action);
    tools_menu->addAction(stop_pvmove_action);

    connect(rescan_action,          SIGNAL(triggered()),
            this, SLOT(reRun()));

    connect(restart_pvmove_action, SIGNAL(triggered()),
            this, SLOT(restartPhysicalVolumeMove()));

    connect(stop_pvmove_action,    SIGNAL(triggered()),
            this, SLOT(stopPhysicalVolumeMove()));

    return tools_menu;
}

KMenu *TopWindow::buildHelpMenu()
{
    KAboutData *const about_data = new KAboutData(QByteArray("kvpm"),
            QByteArray(""),
            ki18n("kvpm"),
            QByteArray("0.9.3"),
            ki18n("Linux volume and partition manager for KDE.  "
                  "This program is still under development, "
                  "bug reports and any comments are welcomed.  "
                  "Licensed under GNU General Public License v3.0"),
            KAboutData::License_GPL_V3,
            ki18n("(c) 2008, 2009, 2010, 2011, 2012 Benjamin Scott"),
            ki18n(""),
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
    KMenu *const file_menu     = new KMenu(i18n("File"));
    KAction *const quit_action =  KStandardAction::quit(qApp, SLOT(quit()), file_menu);
    file_menu->addAction(quit_action);

    return file_menu;
}

KMenu *TopWindow::buildGroupsMenu()
{
    KMenu *const groups_menu   = new KMenu(i18n("Volume Groups"));

    m_remove_vg_action       = new KAction(KIcon("cross"),         i18n("Delete Volume Group..."), this);
    m_reduce_vg_action       = new KAction(KIcon("delete"),        i18n("Reduce Volume Group..."), this);
    m_extend_vg_action       = new KAction(KIcon("add"),           i18n("Extend Volume Group..."), this);
    m_rename_vg_action       = new KAction(KIcon("edit-rename"),   i18n("Rename Volume Group..."), this);
    m_remove_missing_action  = new KAction(KIcon("error_go"),      i18n("Remove Missing Physcial Volumes..."), this);
    m_merge_vg_action        = new KAction(KIcon("arrow_join"),   i18n("Merge Volume Group..."), this);
    m_split_vg_action        = new KAction(KIcon("arrow_divide"), i18n("Split Volume Group..."), this);
    m_change_vg_action       = new KAction(KIcon("wrench"),       i18n("Change Volume Group Attributes..."), this);
    m_create_vg_action       = new KAction(KIcon("document-new"), i18n("Create Volume Group..."), this);
    m_export_vg_action       = new KAction(KIcon("document-export"), i18n("Export Volume Group..."), this);
    m_import_vg_action       = new KAction(KIcon("document-import"), i18n("Import Volume Group..."), this);

    groups_menu->addAction(m_create_vg_action);
    groups_menu->addAction(m_remove_vg_action);
    groups_menu->addAction(m_rename_vg_action);
    groups_menu->addSeparator();
    groups_menu->addAction(m_remove_missing_action);
    groups_menu->addAction(m_extend_vg_action);
    groups_menu->addAction(m_reduce_vg_action);
    groups_menu->addAction(m_split_vg_action);
    groups_menu->addAction(m_merge_vg_action);
    groups_menu->addSeparator();
    groups_menu->addAction(m_export_vg_action);
    groups_menu->addAction(m_import_vg_action);
    groups_menu->addAction(m_change_vg_action);

    connect(m_change_vg_action,  SIGNAL(triggered()),
            this, SLOT(changeVolumeGroup()));

    connect(m_create_vg_action,       SIGNAL(triggered()),
            this, SLOT(createVolumeGroup()));

    connect(m_remove_vg_action,       SIGNAL(triggered()),
            this, SLOT(removeVolumeGroup()));

    connect(m_rename_vg_action,       SIGNAL(triggered()),
            this, SLOT(renameVolumeGroup()));

    connect(m_export_vg_action,       SIGNAL(triggered()),
            this, SLOT(exportVolumeGroup()));

    connect(m_extend_vg_action,       SIGNAL(triggered()),
            this, SLOT(extendVolumeGroup()));

    connect(m_import_vg_action,       SIGNAL(triggered()),
            this, SLOT(importVolumeGroup()));

    connect(m_split_vg_action,       SIGNAL(triggered()),
            this, SLOT(splitVolumeGroup()));

    connect(m_merge_vg_action,       SIGNAL(triggered()),
            this, SLOT(mergeVolumeGroup()));

    connect(m_remove_missing_action,  SIGNAL(triggered()),
            this, SLOT(removeMissingVolumes()));

    connect(m_reduce_vg_action,       SIGNAL(triggered()),
            this, SLOT(reduceVolumeGroup()));

    return groups_menu;
}
