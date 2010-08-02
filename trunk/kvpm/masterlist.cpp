/*
 *
 * 
 * Copyright (C) 2008, 2010 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include <parted/parted.h>

#include <KLocale>
#include <KProgressDialog>

#include <QtGui>

#include "fsprobe.h"
#include "logvol.h"
#include "masterlist.h"
#include "mountentry.h"
#include "mountinfo.h"
#include "physvol.h"
#include "processprogress.h"
#include "storagedevice.h"
#include "volgroup.h"

MasterList::MasterList() : QObject()
{
    rescan();
}

void MasterList::rescan()
{

    KProgressDialog *m_progress_dialog = new KProgressDialog(NULL, i18n("progress"), i18n("scanning volumes"));
    m_progress_dialog->setAllowCancel(false);
    m_progress_dialog->setMinimumDuration(250); 
    QProgressBar *progress_bar = m_progress_dialog->progressBar();
    progress_bar->setRange(0,0);

    lvm_t lvm = lvm_init(NULL);
    lvm_scan(lvm);
    scanVolumeGroups(lvm);
    lvm_quit(lvm);

    scanLogicalVolumes();
    scanStorageDevices();
        
    m_progress_dialog->close();
    m_progress_dialog->delayedDestruct();

    return;
}


/* We can't just clear the lists, we need to delete 
   all the objects they point to  before we can 
   create a brand new list */

MasterList::~MasterList()
{
    for(int x = 0; x < m_volume_groups.size(); x++)
	delete m_volume_groups[x];

    for(int x = 0; x < m_storage_devices.size(); x++)
	delete m_storage_devices[x];
}

void MasterList::scanVolumeGroups(lvm_t lvm)
{
    QStringList vg_output;
    QStringList arguments;
    bool existing_vg;
    bool deleted_vg;

    dm_list *vgnames;
    lvm_str_list *strl;
    
    vgnames = lvm_list_vg_names(lvm);

    dm_list_iterate_items(strl, vgnames){ // rescan() existing VolGroup, don't create a new one
        existing_vg = false;
        for(int x = 0; x < m_volume_groups.size(); x++){
            if( QString(strl->str).trimmed() == m_volume_groups[x]->getName() ){
                existing_vg = true;
                m_volume_groups[x]->rescan(lvm);
            }
        }
        if( !existing_vg )
            m_volume_groups.append( new VolGroup(lvm, strl->str) );
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

void MasterList::scanLogicalVolumes()
{
    QList<LogVol *>  logical_volumes;
    QStringList lv_output;
    QStringList seg_data;
    QStringList arguments;
    QStringList vg_names;
    int lv_count;
    int vg_count;
    int lv_segments;   //number of segments in the logical volume

    for(int x = 0; x < m_volume_groups.size(); x++){
        if( ! m_volume_groups[x]->isExported() )
            vg_names << m_volume_groups[x]->getName();
    }

    MountInformationList mount_info_list;

    if( !vg_names.empty() ){    
        arguments << "lvs" << "--all" << "-o" 
                  << "lv_name,vg_name,lv_attr,lv_size,origin,snap_percent,move_pv,mirror_log,copy_percent,chunksize,seg_count,stripes,stripesize,seg_size,devices,lv_kernel_major,lv_kernel_minor,lv_minor,lv_uuid,vg_fmt,vg_attr,lv_tags"  
                  << "--noheadings" << "--separator" 
                  << "|" << "--nosuffix" 
                  << "--units" << "b" 
                  << "--partial" << vg_names;
    
        ProcessProgress lvscan(arguments, i18n("Scanning logical volumes...") );
        lv_output = lvscan.programOutput();
    
        for(int i = 0; i < lv_output.size(); ) {
            seg_data.clear();
            lv_segments = (lv_output[i].section('|',10,10)).toInt();
            
            for(int x = 0 ; x < lv_segments ; x++){
                seg_data << lv_output[i++];
            }

            logical_volumes.append(new LogVol(seg_data, &mount_info_list));

        } 

        /* put pointers to the lvs in their associated vgs */
    
        lv_count = logical_volumes.size();
        vg_count = m_volume_groups.size();
    
        for(int vg = 0; vg < vg_count; vg++){
            for(int lv = 0; lv < lv_count; lv++){
                if(logical_volumes[lv]->getVolumeGroupName() == m_volume_groups[vg]->getName()){
                    m_volume_groups[vg]->addLogicalVolume(logical_volumes[lv]);
                }
            }    
        }
    }
}
void MasterList::scanLogicalVolumes(VolGroup *VolumeGroup)
{
    LogVol *logical_volume;
    QString group_name;
    QStringList lv_output;
    QStringList seg_data;
    QStringList arguments;
    int lv_segments;   //number of segments in the logical volume
    
    group_name = VolumeGroup->getName();

    MountInformationList mount_info_list;

    arguments << "lvs" << "--all" << "-o" 
	      << "lv_name,vg_name,lv_attr,lv_size,origin,snap_percent,move_pv,mirror_log,copy_percent,chunksize,seg_count,stripes,stripesize,seg_size,devices,lv_kernel_major,lv_kernel_minor,lv_minor,lv_uuid"  
	      << "--noheadings" << "--separator" 
	      << "|" << "--nosuffix" 
	      << "--units" << "b" 
	      << "--partial" << group_name; 
    
    ProcessProgress lvscan(arguments, i18n("Scanning logical volumes...") );
    lv_output = lvscan.programOutput();

    for(int i = 0; i < lv_output.size(); ) {
	seg_data.clear();
	lv_segments = (lv_output[i].section('|',10,10)).toInt();
	
	for(int x = 0 ; x < lv_segments ; x++){
	    seg_data << lv_output[i++];
	}

	logical_volume = new LogVol(seg_data, &mount_info_list);
	VolumeGroup->addLogicalVolume(logical_volume);
	logical_volume->setVolumeGroup(VolumeGroup);
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

