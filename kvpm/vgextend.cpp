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

#include "vgextend.h"

#include <lvm2app.h>

#include <KMessageBox>
#include <KLocale>
#include <KPushButton>
#include <QtGui>

#include "masterlist.h"
#include "misc.h"
#include "pvcheckbox.h"
#include "storagedevice.h"
#include "storagepartition.h"
#include "vgcreate.h"
#include "volgroup.h"

extern MasterList *g_master_list;

bool extend_vg(QString volumeGroupName, StorageDevice *device, StoragePartition *partition)
{
    const QByteArray vg_name = volumeGroupName.toLocal8Bit();
    QByteArray pv_name;
    QString message, error_message;
    long long size;
    lvm_t lvm = g_master_list->getLVM();
    vg_t  vg_dm;
    VolGroup *const vg = g_master_list->getVolGroupByName(volumeGroupName);
    const long long extent_size = vg->getExtentSize();

    if(device){
        size = device->getSize();
        pv_name = device->getName().toLocal8Bit();
    }
    else{
        size = partition->getSize();
        pv_name = partition->getName().toLocal8Bit();
    }

    error_message = i18n("This physical volume <b>%1</b> is smaller than the extent size", QString(pv_name));

    if(extent_size > size){
        KMessageBox::error(0, error_message);
    }
    else{
        message = i18n("Do you want to extend volume group: <b>%1</b> with "
                       "physical volume: <b>%2</b> (size: %3)", 
                       volumeGroupName, 
                       QString(pv_name), 
                       sizeToString(size));

        if( KMessageBox::questionYesNo(0, message) == 3 ){     // 3 is the "yes" button

            if( (vg_dm = lvm_vg_open(lvm, vg_name.data(), "w", 0)) ){
                if( ! lvm_vg_extend(vg_dm, pv_name.data()) ){
                    if( lvm_vg_write(vg_dm) )
                        KMessageBox::error(0, QString(lvm_errmsg(lvm)));;
                    lvm_vg_close(vg_dm);
                    return true;
                }
                KMessageBox::error(0, QString(lvm_errmsg(lvm))); 
                lvm_vg_close(vg_dm);
                return true;
            }
            KMessageBox::error(0, QString(lvm_errmsg(lvm))); 
            return true;
        }
        KMessageBox::error(0, QString(lvm_errmsg(lvm)));
        return true;
    }   

    return false;  // do nothing
}

bool extend_vg(VolGroup *volumeGroup)
{
    QList<StorageDevice *> all_devices = g_master_list->getStorageDevices();   
    QList<StorageDevice *> usable_devices;
    QStringList device_names;
    QList<StoragePartition *> all_partitions;
    QList<StoragePartition *> usable_partitions;

    for(int x = 0; x < all_devices.size(); x++){
        if( (all_devices[x]->getRealPartitionCount() == 0) && 
            (! all_devices[x]->isBusy()) && 
            (! all_devices[x]->isPhysicalVolume() )){
            usable_devices.append(all_devices[x]);
        }
        else if( all_devices[x]->getRealPartitionCount() > 0 ){
            all_partitions = all_devices[x]->getStoragePartitions();
            for(int y = 0; y <all_partitions.size(); y++){
                if( (! all_partitions[y]->isBusy() ) &&
                    (! all_partitions[y]->isPhysicalVolume() ) &&
                    (( all_partitions[y]->isNormal() ) ||  
                     ( all_partitions[y]->isLogical() )))  
                {
                    usable_partitions.append(all_partitions[y]);
                }
            }
        }
    }

    for(int x = usable_devices.size() - 1; x >= 0 ; x--){
        if((usable_devices[x]->getName()).contains("/dev/mapper/") )
            usable_devices.removeAt(x);
    }

    for(int x = usable_partitions.size() - 1; x >= 0 ; x--){
        if( (usable_partitions[x]->getName()).contains("/dev/mapper/") )
            usable_partitions.removeAt(x);
    }

    if( ( usable_devices.size() + usable_partitions.size() ) > 0 ){
        VGExtendDialog dialog( volumeGroup, usable_devices, usable_partitions );
        dialog.exec();
        if(dialog.result() == QDialog::Accepted)
            return true;
        else
            return false;
    }
    else
        KMessageBox::error(0, i18n("No unused potential physical volumes found") );

    return false;
}

VGExtendDialog::VGExtendDialog(VolGroup *volumeGroup, QList<StorageDevice *> devices, 
                               QList<StoragePartition *> partitions, QWidget *parent) : 
    KDialog(parent),
    m_vg(volumeGroup)
{

    setWindowTitle( i18n("Extend Volume Group") );

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    m_layout = new QVBoxLayout();
    dialog_body->setLayout(m_layout);

    QLabel *name_label = new QLabel( i18n("Extending Volume Group <b>%1</b>", m_vg->getName()) );
    name_label->setAlignment(Qt::AlignCenter);
    m_layout->addWidget(name_label);

    m_pv_checkbox = new PVCheckBox( devices, partitions, m_vg->getExtentSize() );
    m_layout->addWidget(m_pv_checkbox);

    connect(m_pv_checkbox, SIGNAL(stateChanged()), 
	    this, SLOT(validateOK()));

    connect(this, SIGNAL(okClicked()), 
	    this, SLOT(commitChanges()));
}

void VGExtendDialog::commitChanges()
{
    const QByteArray vg_name   = m_vg->getName().toLocal8Bit();
    const QStringList pv_names = m_pv_checkbox->getNames();
    QByteArray pv_name;
    lvm_t lvm = g_master_list->getLVM();
    vg_t  vg_dm;

    QEventLoop *loop = new QEventLoop(this);
    QProgressBar *progress_bar = new QProgressBar();
    progress_bar->setRange(0, pv_names.size());
    m_layout->addWidget(progress_bar);
    loop->processEvents();

    if( (vg_dm = lvm_vg_open(lvm, vg_name.data(), "w", 0 )) ){
        
        for(int x = 0; x < pv_names.size(); x++){
            progress_bar->setValue(x);
            loop->processEvents();
            pv_name = pv_names[x].toLocal8Bit();
            if( lvm_vg_extend(vg_dm, pv_name.data()) )
                KMessageBox::error(0, QString(lvm_errmsg(lvm)));
        }
        
        if( lvm_vg_write(vg_dm) )
            KMessageBox::error(0, QString(lvm_errmsg(lvm)));

        lvm_vg_close(vg_dm);
        return;
    }

    KMessageBox::error(0, QString(lvm_errmsg(lvm))); 
    return;
}

void VGExtendDialog::validateOK()
{
    long long space = m_pv_checkbox->getRemainingSpace();

    if(space)
        enableButtonOk(true);
    else
        enableButtonOk(false);
}

