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

#include <lvm2app.h>

#include <KMessageBox>
#include <KLocale>
#include <KPushButton>
#include <QtGui>

#include "masterlist.h"
#include "misc.h"
#include "storagedevice.h"
#include "storagepartition.h"
#include "vgcreate.h"
#include "vgextend.h"
#include "volgroup.h"

extern MasterList *master_list;

bool extend_vg(QString volumeGroupName, QString physicalVolumeName, long long size)
{
    QString message, error_message;
    QStringList args;
    lvm_t  lvm;
    vg_t vg_dm;
    VolGroup *vg =  master_list->getVolGroupByName(volumeGroupName);
    long long extent_size = vg->getExtentSize();

    error_message = i18n("This physical volume <b>%1</b> is smaller than the extent size").arg(physicalVolumeName);

    if(extent_size > size){
        KMessageBox::error(0, error_message);
    }
    else{
        message = i18n("Do you want to extend volume group: <b>%1</b> with "
                       "physical volume: <b>%2</b> (size: %3)").arg(volumeGroupName).arg(physicalVolumeName).arg(sizeToString(size));

        if( KMessageBox::questionYesNo(0, message) == 3 ){     // 3 is the "yes" button
            if( (lvm = lvm_init(NULL)) ){
                if( (vg_dm = lvm_vg_open(lvm, volumeGroupName.toAscii().data(), "w", 0)) ){
                    if( ! lvm_vg_extend(vg_dm, physicalVolumeName.toAscii().data()) ){
                        if( lvm_vg_write(vg_dm) )
                            KMessageBox::error(0, QString(lvm_errmsg(lvm)));;
                        lvm_vg_close(vg_dm);
                        lvm_quit(lvm);
                        return true;
                    }
                    KMessageBox::error(0, QString(lvm_errmsg(lvm))); 
                    lvm_vg_close(vg_dm);
                    lvm_quit(lvm);
                    return true;
                }
                KMessageBox::error(0, QString(lvm_errmsg(lvm))); 
                lvm_quit(lvm);
                return true;
            }
            KMessageBox::error(0, QString(lvm_errmsg(lvm)));
            return true;
        }
    }
    return false;  // do nothing
}

bool extend_vg(VolGroup *volumeGroup)
{
    QList<StorageDevice *> storage_devices = master_list->getStorageDevices();   
    QStringList device_names;
    QList<StoragePartition *> storage_partitions;

    for(int x = 0; x < storage_devices.size(); x++){
        if( (storage_devices[x]->getRealPartitionCount() == 0) && 
            (! storage_devices[x]->isBusy()) && 
            (! storage_devices[x]->isPhysicalVolume() )){
            device_names.append( storage_devices[x]->getDevicePath() );
        }
        else if( storage_devices[x]->getRealPartitionCount() > 0 ){
            storage_partitions = storage_devices[x]->getStoragePartitions();
            for(int y = 0; y <storage_partitions.size(); y++){
                if( (! storage_partitions[y]->isBusy() ) &&
                    (! storage_partitions[y]->isPhysicalVolume() ) &&
                    (( storage_partitions[y]->isNormal() ) ||  
                     ( storage_partitions[y]->isLogical() )))  
                {
                    device_names.append( storage_partitions[y]->getName() );
                }
            }
        }
    }

    for(int x = device_names.size() - 1; x >= 0 ; x--){
        if( device_names[x].contains("/dev/mapper/") )
            device_names.removeAt(x);
    }

    if( device_names.size() > 0 ){
        VGExtendDialog dialog( volumeGroup, device_names );
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

VGExtendDialog::VGExtendDialog(VolGroup *volumeGroup, QStringList physicalVolumeNames, QWidget *parent) : 
    KDialog(parent), m_pv_names(physicalVolumeNames), m_vg(volumeGroup)
{

    setWindowTitle( i18n("Extend Volume Group") );

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    m_layout = new QVBoxLayout();
    dialog_body->setLayout(m_layout);

    QLabel *name_label = new QLabel( i18n("Extending Volume Group <b>%1</b>").arg(m_vg->getName()) );
    name_label->setAlignment(Qt::AlignCenter);
    m_layout->addWidget(name_label);

    QGroupBox *pv_box = new QGroupBox( i18n("Available potential physical volumes") ); 
    QGridLayout *pv_box_layout = new QGridLayout();
    pv_box->setLayout(pv_box_layout);
    int pv_check_count = m_pv_names.size();
    NoMungeCheck *temp_check;

    if(m_pv_names.size() < 2){
        pv_box_layout->addWidget( new QLabel(m_pv_names[0] ) );
        enableButtonOk(true);
    }
    else{
        for(int x = 0; x < m_pv_names.size(); x++){
	    temp_check = new NoMungeCheck(m_pv_names[x]);
	    m_pv_checks.append(temp_check);

            if(pv_check_count < 11 )
                pv_box_layout->addWidget(m_pv_checks[x], x % 5, x / 5);
            else if (pv_check_count % 3 == 0)
                pv_box_layout->addWidget(m_pv_checks[x], x % (pv_check_count / 3), x / (pv_check_count / 3));
            else
                pv_box_layout->addWidget(m_pv_checks[x], x % ( (pv_check_count + 2) / 3), x / ( (pv_check_count + 2) / 3));

            connect(temp_check, SIGNAL(toggled(bool)), 
                    this, SLOT(validateOK()));
        }
        enableButtonOk(false);
    }    

    QHBoxLayout *button_layout = new QHBoxLayout();
    pv_box_layout->addLayout(button_layout, pv_box_layout->rowCount(),0, 1, -1);
    KPushButton *all_button = new KPushButton( i18n("Select all") );
    KPushButton *none_button = new KPushButton( i18n("Select none") );

    connect(all_button, SIGNAL(clicked(bool)), 
            this, SLOT(selectAll()));
    connect(none_button, SIGNAL(clicked(bool)), 
            this, SLOT(selectNone()));


    button_layout->addWidget(all_button);
    button_layout->addWidget(none_button);

    m_layout->addWidget(pv_box);

    connect(this, SIGNAL(okClicked()), 
	    this, SLOT(commitChanges()));
}

void VGExtendDialog::commitChanges()
{
    lvm_t  lvm;
    vg_t vg_dm;

    if( m_pv_checks.size() > 1 ){
        m_pv_names.clear();	
        for(int x = 0; x < m_pv_checks.size(); x++){
	    if( m_pv_checks[x]->isChecked() )
	        m_pv_names.append( m_pv_checks[x]->getUnmungedText() );
	}
    }

    QEventLoop *loop = new QEventLoop(this);
    QProgressBar *progress_bar = new QProgressBar();
    progress_bar->setRange(0, m_pv_names.size());
    m_layout->addWidget(progress_bar);
    loop->processEvents();

    if( (lvm = lvm_init(NULL)) ){
        if( (vg_dm = lvm_vg_open(lvm, m_vg->getName().toAscii().data(), "w", 0 )) ){

            for(int x = 0; x < m_pv_names.size(); x++){
	        progress_bar->setValue(x);
	        loop->processEvents();
                if( lvm_vg_extend(vg_dm, m_pv_names[x].toAscii().data()) )
                    KMessageBox::error(0, QString(lvm_errmsg(lvm)));
            }

            if( lvm_vg_write(vg_dm) )
                KMessageBox::error(0, QString(lvm_errmsg(lvm)));
            lvm_vg_close(vg_dm);
            lvm_quit(lvm);
            return;
        }
        KMessageBox::error(0, QString(lvm_errmsg(lvm))); 
        lvm_quit(lvm);
        return;
    }
    KMessageBox::error(0, QString(lvm_errmsg(lvm))); 
    return;
}

void VGExtendDialog::validateOK()
{
    enableButtonOk(false);
    for(int x = 0; x < m_pv_checks.size(); x++){
        if(m_pv_checks[x]->isChecked()){
            enableButtonOk(true);
            break;
        }
    }
}

void VGExtendDialog::selectAll()
{
  for(int x = 0; x < m_pv_checks.size(); x++)
    m_pv_checks[x]->setChecked(true);
}

void VGExtendDialog::selectNone()
{
  for(int x = 0; x < m_pv_checks.size(); x++)
    m_pv_checks[x]->setChecked(false);
}
