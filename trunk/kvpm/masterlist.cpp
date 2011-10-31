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
#include <KProgressDialog>
#include <KMessageBox>

#include <QtGui>

#include "fsprobe.h"
#include "logvol.h"
#include "mountentry.h"
#include "mountinfolist.h"
#include "pedexceptions.h"
#include "physvol.h"
#include "processprogress.h"
#include "storagedevice.h"
#include "volgroup.h"


MasterList::MasterList() : QObject()
{
    ped_exception_set_handler(my_handler);
    m_lvm = lvm_init(NULL);
}

/* We can't just clear the lists, we need to delete 
   all the objects they point to  before we can 
   create a brand new list */

MasterList::~MasterList()
{

    lvm_quit(m_lvm);

    for(int x = 0; x < m_volume_groups.size(); x++)
	delete m_volume_groups[x];

    for(int x = 0; x < m_storage_devices.size(); x++)
	delete m_storage_devices[x];
}

void MasterList::rescan()
{

    KProgressDialog *progress_dialog = new KProgressDialog(NULL, i18n("progress"), i18n("scanning LVM"));
    progress_dialog->setAllowCancel(false);
    progress_dialog->setMinimumDuration(250); 
    QProgressBar *progress_bar = progress_dialog->progressBar();
    progress_bar->setRange(0,3);
    progress_dialog->show();
    progress_dialog->ensurePolished();
    qApp->processEvents();

    lvm_scan(m_lvm);
    lvm_config_reload(m_lvm);    

    progress_bar->setValue(1);
    progress_dialog->setLabelText( i18n("scanning volume groups") );
    progress_dialog->ensurePolished();
    qApp->processEvents();
    scanVolumeGroups();

    progress_bar->setValue(2);
    progress_dialog->setLabelText( i18n("scanning storage devices") );
    progress_dialog->ensurePolished();
    qApp->processEvents();
    scanStorageDevices();

    qApp->processEvents();
    progress_dialog->close();
    progress_dialog->delayedDestruct();

    return;
}

void MasterList::scanVolumeGroups()
{
    QStringList vg_output;
    QStringList arguments;
    bool existing_vg;
    bool deleted_vg;

    dm_list *vgnames;
    lvm_str_list *strl;

    vgnames = lvm_list_vg_names(m_lvm);
    dm_list_iterate_items(strl, vgnames){ // rescan() existing VolGroup, don't create a new one
        existing_vg = false;
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
        deleted_vg = true;
        dm_list_iterate_items(strl, vgnames){ 
            if( QString(strl->str).trimmed() == m_volume_groups[x]->getName() )
                deleted_vg = false;
        }
        if(deleted_vg){
            delete m_volume_groups.takeAt(x);
        }
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

