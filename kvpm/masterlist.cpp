/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "masterlist.h"

#include "dmraid.h"
#include "fsprobe.h"
#include "logvol.h"
#include "mountentry.h"
#include "mounttables.h"
#include "pedexceptions.h"
#include "pedthread.h"
#include "physvol.h"
#include "processprogress.h"
#include "progressbox.h"
#include "storagedevice.h"
#include "topwindow.h"
#include "volgroup.h"

#include <parted/parted.h>

#include <QDebug>
#include <QElapsedTimer>
#include <QSemaphore>
#include <QThread>

#include <KApplication>
#include <KLocale>
#include <KMessageBox>



// These are static being variables initialized here

QList<VolGroup *> MasterList::m_volume_groups = QList<VolGroup *>();
QList<StorageDevice *> MasterList::m_storage_devices = QList<StorageDevice *>();
lvm_t MasterList::m_lvm = NULL;

int MasterList::m_LvmVersionMajor = 0;
int MasterList::m_LvmVersionMinor = 0;
int MasterList::m_LvmVersionPatchLevel = 0;
int MasterList::m_LvmVersionApi = 0;

MasterList::MasterList() : QObject()
{
    m_lvm = lvm_init(NULL);

    QStringList version = QString(lvm_library_get_version()).split('.');
    m_LvmVersionMajor = version[0].toInt();
    m_LvmVersionMinor = version[1].toInt();
    version = version[2].split(QRegExp("[()]"));
    m_LvmVersionPatchLevel = version[0].toInt();
    m_LvmVersionApi = version[1].toInt();

    ped_exception_set_handler(my_handler);
    m_mount_tables = new MountTables();
}

MasterList::~MasterList()
{
    if (m_lvm)
        lvm_quit(m_lvm);
}

void MasterList::rescan()
{
    ProgressBox *progress_box = TopWindow::getProgressBox();
    m_mount_tables->loadData();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    progress_box->setRange(0, 3);
    progress_box->setValue(0);
    progress_box->setText("Scan system");
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    // The lvm and PED scan can both be slow so we
    // run the PED system scan in a background thread
    QSemaphore semaphore(1);
    PedThread ped_thread(&semaphore);
    ped_thread.start();
    lvm_scan(m_lvm);
    lvm_config_reload(m_lvm);
    semaphore.acquire(1);      // wait for thread to finish

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    progress_box->setValue(1);
    progress_box->setText("Scan vgs");
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    scanVolumeGroups();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    progress_box->setValue(2);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    progress_box->setText("Scan devs");
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    scanStorageDevices();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    progress_box->reset();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    return;
}

void MasterList::scanVolumeGroups()
{
    dm_list *vgnames;
    lvm_str_list *strl;

    vgnames = lvm_list_vg_names(m_lvm);
    dm_list_iterate_items(strl, vgnames) { // rescan() existing VolGroup, don't create a new one
        bool existing_vg = false;
        for (int x = 0; x < m_volume_groups.size(); x++) {
            if (QString(strl->str).trimmed() == m_volume_groups[x]->getName()) {
                existing_vg = true;
                m_volume_groups[x]->rescan(m_lvm);
            }
        }
        if (!existing_vg)
            m_volume_groups.append(new VolGroup(m_lvm, strl->str, m_mount_tables));
    }
    for (int x = m_volume_groups.size() - 1; x >= 0; x--) { // delete VolGroup if the vg is gone
        bool deleted_vg = true;
        dm_list_iterate_items(strl, vgnames) {
            if (QString(strl->str).trimmed() == m_volume_groups[x]->getName())
                deleted_vg = false;
        }
        if (deleted_vg)
            delete m_volume_groups.takeAt(x);
    }
}

void MasterList::scanStorageDevices()
{
    QList<PhysVol *>  physical_volumes;

    for (auto vg : m_volume_groups)
        physical_volumes << vg->getPhysicalVolumes();

    for (auto sd : m_storage_devices)
        delete sd;

    m_storage_devices.clear();
    PedDevice *dev = NULL;
    QStringList block, raid;
    dmraid_get_devices(block, raid);

    while ((dev = ped_device_get_next(dev))) {
        if (!QString(dev->path).startsWith("/dev/mapper"))
            m_storage_devices.append(new StorageDevice(dev, physical_volumes, m_mount_tables, block, raid));
        else if (raid.contains(dev->path))
            m_storage_devices.prepend(new StorageDevice(dev, physical_volumes, m_mount_tables, block, raid));
    }
}

int MasterList::getVgCount()
{
    return m_volume_groups.size();
}

QList<VolGroup *> MasterList::getVolGroups()
{
    return m_volume_groups;
}

QList<StorageDevice *> MasterList::getStorageDevices()
{
    return m_storage_devices;
}

lvm_t MasterList::getLvm()
{
    return m_lvm;
}

VolGroup* MasterList::getVgByName(QString name)
{
    name = name.trimmed();

    for (int x = 0; x < m_volume_groups.size(); x++) {
        if (name == m_volume_groups[x]->getName())
            return m_volume_groups[x];
    }

    return NULL;
}

QStringList MasterList::getVgNames()
{
    QStringList names;

    for (int x = 0; x < m_volume_groups.size(); x++)
        names << m_volume_groups[x]->getName();

    return names;
}

int MasterList::getLvmVersionMajor()
{
    return m_LvmVersionMajor;
}

int MasterList::getLvmVersionMinor()
{
    return m_LvmVersionMinor;
}

int MasterList::getLvmVersionPatchLevel()
{
    return m_LvmVersionPatchLevel;
}

int MasterList::getLvmVersionApi()
{
    return m_LvmVersionApi;
}
