/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
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

#include <parted/parted.h>

#include <KLocale>
#include <KMessageBox>

#include <QtGui>

#include "fsprobe.h"
#include "logvol.h"
#include "mountentry.h"
#include "mountinfolist.h"
#include "pedexceptions.h"
#include "physvol.h"
#include "processprogress.h"
#include "progressbox.h"
#include "storagedevice.h"
#include "topwindow.h"
#include "volgroup.h"


// These are static being variables initialized here

QList<VolGroup *> MasterList::m_volume_groups = QList<VolGroup *>();
QList<StorageDevice *> MasterList::m_storage_devices = QList<StorageDevice *>();
lvm_t MasterList::m_lvm = lvm_init(NULL);


MasterList::MasterList() : QObject()
{
    ped_exception_set_handler(my_handler);
}

MasterList::~MasterList()
{
    lvm_quit(m_lvm);
}

void MasterList::rescan()
{
    ProgressBox *progress_box = TopWindow::getProgressBox();

    progress_box->setRange(0,3);
    progress_box->setText("Scan lvm");
    qApp->setOverrideCursor(Qt::WaitCursor);
    qApp->processEvents();

    lvm_scan(m_lvm);
    lvm_config_reload(m_lvm);    

    progress_box->setValue(1);
    progress_box->setText("Scan vgs");
    qApp->processEvents();
    scanVolumeGroups();

    progress_box->setValue(2);
    qApp->processEvents();
    progress_box->setText("Scan devs");
    scanStorageDevices();

    progress_box->setValue(3);
    qApp->restoreOverrideCursor();
    qApp->processEvents();

    return;
}

void MasterList::scanVolumeGroups()
{
    dm_list *vgnames;
    lvm_str_list *strl;

    vgnames = lvm_list_vg_names(m_lvm);
    dm_list_iterate_items(strl, vgnames){ // rescan() existing VolGroup, don't create a new one
        bool existing_vg = false;
        for(int x = 0; x < m_volume_groups.size(); x++){
            if( QString(strl->str).trimmed() == m_volume_groups[x]->getName() ){
                existing_vg = true;
                m_volume_groups[x]->rescan(m_lvm);
            }
        }
        if( !existing_vg )
            m_volume_groups.append( new VolGroup(m_lvm, strl->str) );
    }

    for(int x = m_volume_groups.size() - 1; x >= 0; x--){ // delete VolGroup if the vg is gone
        bool deleted_vg = true;
        dm_list_iterate_items(strl, vgnames){ 
            if( QString(strl->str).trimmed() == m_volume_groups[x]->getName() )
                deleted_vg = false;
        }
        if(deleted_vg)
            delete m_volume_groups.takeAt(x);
    }
}

void MasterList::scanStorageDevices()
{
    PedDevice *dev = NULL;
    QList<PhysVol *>  physical_volumes;

    for(int x = 0; x < m_volume_groups.size(); x++)
        physical_volumes.append( m_volume_groups[x]->getPhysicalVolumes() );

    for(int x = 0; x < m_storage_devices.size(); x++)
	delete m_storage_devices[x];

    m_storage_devices.clear();

    ped_device_free_all();
    ped_device_probe_all();

    MountInformationList *mount_info_list = new MountInformationList();

    while( ( dev = ped_device_get_next(dev) ) ){
        if( !QString("%1").arg(dev->path).contains("/dev/mapper") )
            m_storage_devices.append( new StorageDevice(dev, physical_volumes, mount_info_list ) );
    }
}

int MasterList::getVolGroupCount()
{
      return m_volume_groups.size();
}

const QList<VolGroup *> MasterList::getVolGroups()
{
      return m_volume_groups;
}

const QList<StorageDevice *> MasterList::getStorageDevices()
{
    return m_storage_devices;
}

lvm_t MasterList::getLVM()
{
    return m_lvm;
}

VolGroup* MasterList::getVolGroupByName(QString name)
{
    name = name.trimmed();

    for(int x = 0; x < m_volume_groups.size(); x++){
	if(name == m_volume_groups[x]->getName())
	    return m_volume_groups[x];
    }

    return NULL;
}

QStringList MasterList::getVolumeGroupNames()
{
    QStringList names;

    for(int x = 0; x < m_volume_groups.size(); x++)
	names << m_volume_groups[x]->getName();

    return names;
}

