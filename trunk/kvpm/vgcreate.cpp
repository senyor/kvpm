/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include "vgcreate.h"

#include <lvm2app.h>

#include <KLocale>
#include <KMessageBox>
#include <KProgressDialog>
#include <KPushButton>
#include <KSeparator>
#include <QtGui>

#include "masterlist.h"
#include "pvcheckbox.h"
#include "storagedevice.h"
#include "storagepartition.h"


extern MasterList *g_master_list;

bool create_vg()
{
    QList<StorageDevice *> storage_devices = g_master_list->getStorageDevices();   
    QList<StorageDevice *> usable_devices;
    QList<StoragePartition *> storage_partitions;
    QList<StoragePartition *> usable_partitions;

    for(int x = 0; x < storage_devices.size(); x++){
        if( (storage_devices[x]->getRealPartitionCount() == 0) && 
            (! storage_devices[x]->isBusy()) && 
            (! storage_devices[x]->isPhysicalVolume() )) {

                usable_devices.append( storage_devices[x] );
        }
        else if( storage_devices[x]->getRealPartitionCount() > 0 ){
            storage_partitions = storage_devices[x]->getStoragePartitions();
            for(int y = 0; y < storage_partitions.size(); y++){
                if( (! storage_partitions[y]->isBusy() ) && (! storage_partitions[y]->isPhysicalVolume() ) &&
                    (( storage_partitions[y]->isNormal() ) || ( storage_partitions[y]->isLogical() )))  {

                    usable_partitions.append(storage_partitions[y]); 
                }
            }
        }
    }

    if( usable_partitions.size() + usable_devices.size() > 0 ){
        VGCreateDialog dialog( usable_devices, usable_partitions );
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

bool create_vg(StorageDevice *device, StoragePartition *partition){

    QList<StorageDevice *>    devices;
    QList<StoragePartition *> partitions;

    if(device)
        devices.append(device);
    else if(partition)
        partitions.append(partition);

    VGCreateDialog dialog(devices, partitions);
    dialog.exec();
    
    if(dialog.result() == QDialog::Accepted)
        return true;
    else
        return false;

    return true;
}

VGCreateDialog::VGCreateDialog(QList<StorageDevice *> devices, QList<StoragePartition *> partitions, QWidget *parent) : 
    KDialog(parent) 
{
    setWindowTitle( i18n("Create Volume Group") );

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    m_layout = new QVBoxLayout();
    dialog_body->setLayout(m_layout);

    QLabel *name_label = new QLabel( i18n("Volume Group Name: ") );
    m_vg_name = new KLineEdit();

    QRegExp rx("[0-9a-zA-Z_\\.][-0-9a-zA-Z_\\.]*");
    m_validator = new QRegExpValidator( rx, m_vg_name );
    m_vg_name->setValidator(m_validator);
    QHBoxLayout *name_layout = new QHBoxLayout();
    name_layout->addWidget(name_label);
    name_layout->addWidget(m_vg_name);

    QLabel *extent_label = new QLabel( i18n("Physical Extent Size: ") );
    m_extent_size = new KComboBox();
    m_extent_size->insertItem(0,"1");
    m_extent_size->insertItem(1,"2");
    m_extent_size->insertItem(2,"4");
    m_extent_size->insertItem(3,"8");
    m_extent_size->insertItem(4,"16");
    m_extent_size->insertItem(5,"32");
    m_extent_size->insertItem(6,"64");
    m_extent_size->insertItem(7,"128");
    m_extent_size->insertItem(8,"256");
    m_extent_size->insertItem(9,"512");
    m_extent_size->setInsertPolicy(QComboBox::NoInsert);
    m_extent_size->setCurrentIndex(2);
    m_extent_suffix = new KComboBox();
    m_extent_suffix->insertItem(0,"KiB");
    m_extent_suffix->insertItem(1,"MiB");
    m_extent_suffix->insertItem(2,"GiB");
    m_extent_suffix->setInsertPolicy(QComboBox::NoInsert);
    m_extent_suffix->setCurrentIndex(1);
    
    m_pv_checkbox = new PVCheckBox(devices, partitions, 0x400000);  // 4 MiB default extent size
    m_layout->addWidget(m_pv_checkbox);

    connect(m_pv_checkbox, SIGNAL(stateChanged()),
            this, SLOT(validateOK()));

    QHBoxLayout *extent_layout = new QHBoxLayout();
    extent_layout->addWidget(extent_label);
    extent_layout->addWidget(m_extent_size);
    extent_layout->addWidget(m_extent_suffix);

    /*  liblvm does not support setting limits on lvs and pvs yet (will it ever?)

    QGroupBox *lv_box = new QGroupBox( i18n("Number of Logical Volumes") );
    QVBoxLayout *lv_layout_v = new QVBoxLayout();
    QHBoxLayout *lv_layout_h = new QHBoxLayout();
    lv_box->setLayout(lv_layout_v);
    m_max_lvs_check = new QCheckBox( i18n("No Limit") );
    m_max_lvs_check->setCheckState(Qt::Checked);
    lv_layout_v->addWidget(m_max_lvs_check);
    lv_layout_v->addLayout(lv_layout_h);
    QLabel *lv_label = new QLabel( i18n("Maximum: ") );
    m_max_lvs = new KLineEdit();
    QIntValidator *lv_validator = new QIntValidator(1,255,this);
    m_max_lvs->setValidator(lv_validator);
    m_max_lvs->setEnabled(false);
    lv_layout_h->addWidget(lv_label);
    lv_layout_h->addWidget(m_max_lvs);

    QGroupBox *pv_box = new QGroupBox( i18n("Number of Physical Volumes") );
    QVBoxLayout *pv_layout_v = new QVBoxLayout();
    QHBoxLayout *pv_layout_h = new QHBoxLayout();
    pv_box->setLayout(pv_layout_v);
    m_max_pvs_check = new QCheckBox( i18n("No Limit") );
    m_max_pvs_check->setCheckState(Qt::Checked);
    pv_layout_v->addWidget(m_max_pvs_check);
    pv_layout_v->addLayout(pv_layout_h);
    QLabel *pv_label = new QLabel( i18n("Maximum: ") );
    m_max_pvs = new KLineEdit();
    QIntValidator *pv_validator = new QIntValidator(1,255,this);
    m_max_pvs->setValidator(pv_validator);
    m_max_pvs->setEnabled(false);
    pv_layout_h->addWidget(pv_label);
    pv_layout_h->addWidget(m_max_pvs);

    */

    m_clustered = new QCheckBox( i18n("Cluster Aware") );
    m_clustered->setEnabled(false);

    m_auto_backup = new QCheckBox( i18n("Automatic Backup") );
    m_auto_backup->setCheckState(Qt::Checked);
    m_auto_backup->setEnabled(false);

    m_layout->addLayout(name_layout);
    m_layout->addLayout(extent_layout);
    //    m_layout->addWidget(lv_box);
    //    m_layout->addWidget(pv_box);
    m_layout->addWidget(m_clustered);
    m_layout->addWidget(m_auto_backup);
    m_layout->addWidget(new KSeparator);
    m_progress_bar = new QProgressBar();
    m_layout->addWidget(m_progress_bar, Qt::AlignCenter);

    enableButtonOk(false);

    connect(m_vg_name, SIGNAL(textChanged(QString)), 
	    this, SLOT(validateOK()));
    /*
    connect(m_max_lvs_check, SIGNAL(stateChanged(int)), 
	    this, SLOT(limitLogicalVolumes(int)));

    connect(m_max_pvs_check, SIGNAL(stateChanged(int)), 
	    this, SLOT(limitPhysicalVolumes(int)));
    */
    connect(this, SIGNAL(okClicked()), 
	    this, SLOT(commitChanges()));

    connect(m_extent_size, SIGNAL(activated(int)), 
            this, SLOT(extentSizeChanged()));

    connect(m_extent_suffix, SIGNAL(activated(int)), 
            this, SLOT(extentSizeChanged()));

}

void VGCreateDialog::extentSizeChanged(){

    limitExtentSize(m_extent_suffix->currentIndex() );

    uint32_t new_extent_size = m_extent_size->currentText().toULong();

    new_extent_size *= 1024;
    if( m_extent_suffix->currentIndex() > 0 )
        new_extent_size *= 1024;
    if( m_extent_suffix->currentIndex() > 1 )
        new_extent_size *= 1024;

    m_pv_checkbox->setExtentSize(new_extent_size);
}

void VGCreateDialog::limitExtentSize(int index){

    int extent_index;

    if( index > 1 ){  // Gigabytes selected as suffix, more than 2Gib forbidden
        if( m_extent_size->currentIndex() > 2 )
            m_extent_size->setCurrentIndex(0);
        m_extent_size->setMaxCount(2);
    }
    else{
        extent_index = m_extent_size->currentIndex();
        m_extent_size->setMaxCount(10);
        m_extent_size->setInsertPolicy(QComboBox::InsertAtBottom);
        m_extent_size->insertItem(2,"4");
        m_extent_size->insertItem(3,"8");
        m_extent_size->insertItem(4,"16");
        m_extent_size->insertItem(5,"32");
        m_extent_size->insertItem(6,"64");
        m_extent_size->insertItem(7,"128");
        m_extent_size->insertItem(8,"256");
        m_extent_size->insertItem(9,"512");
        m_extent_size->setInsertPolicy(QComboBox::NoInsert);
        m_extent_size->setCurrentIndex(extent_index);
    }
}

void VGCreateDialog::commitChanges()
{
    lvm_t lvm = g_master_list->getLVM();
    vg_t vg_dm;
    uint32_t new_extent_size = m_extent_size->currentText().toULong();
    const QStringList pv_names = m_pv_checkbox->getNames();
    const QByteArray vg_name_array = m_vg_name->text().toLocal8Bit();
    QByteArray pv_name_array;

    new_extent_size *= 1024;
    if( m_extent_suffix->currentIndex() > 0 )
        new_extent_size *= 1024;
    if( m_extent_suffix->currentIndex() > 1 )
        new_extent_size *= 1024;

    QEventLoop *loop = new QEventLoop(this);
    m_progress_bar->setRange(0, pv_names.size());
    loop->processEvents();

    if( (vg_dm = lvm_vg_create(lvm, vg_name_array.data())) ){

        if( (lvm_vg_set_extent_size(vg_dm, new_extent_size)) )
            KMessageBox::error(0, QString(lvm_errmsg(lvm)));
        
        for(int x = 0; x < pv_names.size(); x++){
            m_progress_bar->setValue(x);
            loop->processEvents();
            pv_name_array = pv_names[x].toLocal8Bit();
            if( lvm_vg_extend(vg_dm, pv_name_array.data()) )
                KMessageBox::error(0, QString(lvm_errmsg(lvm)));
        }
        
            // ****To Do... None of the following are supported by liblvm2app yet****
            //   if(m_clustered->isChecked())
            //   if(m_auto_backup->isChecked())          
            //   if((m_max_lvs_check->isChecked()) && (m_max_lvs->text() != ""))
            //   if((m_max_pvs_check->isChecked()) && (m_max_pvs->text() != ""))


        if( lvm_vg_write(vg_dm) )
            KMessageBox::error(0, QString(lvm_errmsg(lvm)));
        
        lvm_vg_close(vg_dm);
        delete loop;
        return;
    }
        
    KMessageBox::error(0, QString(lvm_errmsg(lvm))); 
    delete loop;
    return;
}

/* The allowed characters in the name are letters, numbers, periods
   hyphens and underscores. Also, the names ".", ".." and names starting
   with a hyphen are disallowed. Finally disable the OK button if no 
   pvs are checked  */

void VGCreateDialog::validateOK()
{
    QString name = m_vg_name->text();
    int pos = 0;
    long long space = m_pv_checkbox->getRemainingSpace();

    enableButtonOk(false);

    if(m_validator->validate(name, pos) == QValidator::Acceptable && name != "." && name != ".."){
        if(space)
            enableButtonOk(true);
    }
}
/*
void VGCreateDialog::limitLogicalVolumes(int boxstate)
{
    if(boxstate == Qt::Unchecked)
	m_max_lvs->setEnabled(true);
    else
	m_max_lvs->setEnabled(false);
}

void VGCreateDialog::limitPhysicalVolumes(int boxstate)
{
    if(boxstate == Qt::Unchecked)
	m_max_pvs->setEnabled(true);
    else
	m_max_pvs->setEnabled(false);
}
*/
