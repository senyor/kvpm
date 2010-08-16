/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010 Benjamin Scott   <benscott@nwlink.com>
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

#include <KLocale>
#include <KMessageBox>
#include <KProgressDialog>
#include <KPushButton>
#include <QtGui>

#include "storagedevice.h"
#include "storagepartition.h"
#include "vgcreate.h"
#include "masterlist.h"

extern MasterList *master_list;

bool create_vg()
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
                    (! storage_partitions[y]->isPV() ) &&
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
        VGCreateDialog dialog( device_names );
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

bool create_vg(QString physicalVolumePath){

    QStringList physicalVolumePathList(physicalVolumePath);

    VGCreateDialog dialog(physicalVolumePathList);
    dialog.exec();
    
    if(dialog.result() == QDialog::Accepted)
        return true;
    else
        return false;
}

VGCreateDialog::VGCreateDialog(QStringList physicalVolumePathList, QWidget *parent) : 
  KDialog(parent), m_pv_paths(physicalVolumePathList)
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

    QGroupBox *new_pv_box = new QGroupBox( i18n("Available potential physical volumes") ); 
    QGridLayout *new_pv_box_layout = new QGridLayout();
    new_pv_box->setLayout(new_pv_box_layout);
    NoMungeCheck *temp_check;
    int pv_check_count = m_pv_paths.size();

    if(pv_check_count < 2)
        new_pv_box_layout->addWidget( new QLabel(m_pv_paths[0] ) );
    else{
        for(int x = 0; x < pv_check_count; x++){
	    temp_check = new NoMungeCheck(m_pv_paths[x]);
	    m_pv_checks.append(temp_check);

            if(pv_check_count < 11 )
                new_pv_box_layout->addWidget(m_pv_checks[x], x % 5, x / 5);
            else if (pv_check_count % 3 == 0)
                new_pv_box_layout->addWidget(m_pv_checks[x], x % (pv_check_count / 3), x / (pv_check_count / 3));
            else
                new_pv_box_layout->addWidget(m_pv_checks[x], x % ( (pv_check_count + 2) / 3), x / ( (pv_check_count + 2) / 3));

	    connect(temp_check, SIGNAL(toggled(bool)), 
		    this, SLOT(validateOK()));
	}
    }
    QHBoxLayout *button_layout = new QHBoxLayout();
    new_pv_box_layout->addLayout(button_layout, new_pv_box_layout->rowCount(),0, 1, -1);
    KPushButton *all_button = new KPushButton( i18n("Select all") );
    KPushButton *none_button = new KPushButton( i18n("Select none") );

    connect(all_button, SIGNAL(clicked(bool)), 
            this, SLOT(selectAll()));
    connect(none_button, SIGNAL(clicked(bool)), 
            this, SLOT(selectNone()));


    button_layout->addWidget(all_button);
    button_layout->addWidget(none_button);

    QHBoxLayout *extent_layout = new QHBoxLayout();
    extent_layout->addWidget(extent_label);
    extent_layout->addWidget(m_extent_size);
    extent_layout->addWidget(m_extent_suffix);

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

    m_clustered = new QCheckBox( i18n("Cluster Aware") );
    m_clustered->setEnabled(false);
    
    m_auto_backup = new QCheckBox( i18n("Automatic Backup") );
    m_auto_backup->setCheckState(Qt::Checked);

    m_layout->addLayout(name_layout);
    m_layout->addLayout(extent_layout);
    m_layout->addWidget(new_pv_box);
    m_layout->addWidget(lv_box);
    m_layout->addWidget(pv_box);
    m_layout->addWidget(m_clustered);
    m_layout->addWidget(m_auto_backup);

    enableButtonOk(false);

    connect(m_vg_name, SIGNAL(textChanged(QString)), 
	    this, SLOT(validateOK()));

    connect(m_max_lvs_check, SIGNAL(stateChanged(int)), 
	    this, SLOT(limitLogicalVolumes(int)));

    connect(m_max_pvs_check, SIGNAL(stateChanged(int)), 
	    this, SLOT(limitPhysicalVolumes(int)));

    connect(this, SIGNAL(okClicked()), 
	    this, SLOT(commitChanges()));

    connect(m_extent_suffix, SIGNAL(activated(int)), 
            this, SLOT(limitExtentSize(int)));

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
    lvm_t  lvm;
    vg_t vg_dm;
    uint32_t new_extent_size = m_extent_size->currentText().toULong();

    new_extent_size *= 1024;
    if( m_extent_suffix->currentIndex() > 0 )
        new_extent_size *= 1024;
    if( m_extent_suffix->currentIndex() > 1 )
        new_extent_size *= 1024;

    if( m_pv_checks.size() > 1 ){
        m_pv_paths.clear();	
        for(int x = 0; x < m_pv_checks.size(); x++){
	    if( m_pv_checks[x]->isChecked() )
	        m_pv_paths.append( m_pv_checks[x]->getUnmungedText() );
	}
    }

    QEventLoop *loop = new QEventLoop(this);
    QProgressBar *progress_bar = new QProgressBar();
    progress_bar->setRange(0, m_pv_paths.size());
    m_layout->addWidget(progress_bar);
    loop->processEvents();

    if( (lvm = lvm_init(NULL)) ){
        if( (vg_dm = lvm_vg_create(lvm, m_vg_name->text().toAscii().data())) ){

            if( (lvm_vg_set_extent_size(vg_dm, new_extent_size)) )
                KMessageBox::error(0, QString(lvm_errmsg(lvm)));

            for(int x = 0; x < m_pv_paths.size(); x++){
                progress_bar->setValue(x);
                loop->processEvents();
                if( lvm_vg_extend(vg_dm, m_pv_paths[x].toAscii().data()) )
                    KMessageBox::error(0, QString(lvm_errmsg(lvm)));
            }

            // ****To Do... None of the following are supported by liblvm2app yet****
            //   if(m_clustered->isChecked())
            //   if(m_auto_backup->isChecked())          
            //   if((!m_max_lvs_check->isChecked()) && (m_max_lvs->text() != ""))
            //   if((!m_max_pvs_check->isChecked()) && (m_max_pvs->text() != ""))

            if( lvm_vg_write(vg_dm) )
                KMessageBox::error(0, QString(lvm_errmsg(lvm)));
            lvm_vg_close(vg_dm);
            lvm_quit(lvm);
            return;
        }
        lvm_quit(lvm);
        KMessageBox::error(0, QString(lvm_errmsg(lvm))); 
        return;
    }
    KMessageBox::error(0, QString(lvm_errmsg(lvm))); 
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

    enableButtonOk(false);

    if(m_validator->validate(name, pos) == QValidator::Acceptable && name != "." && name != ".."){

        if( ! m_pv_checks.size() )        // if there is only one pv possible, there is no list
	        enableButtonOk(true);
	else{
	    for(int x = 0; x < m_pv_checks.size(); x++){
	        if( m_pv_checks[x]->isChecked() ){
		    enableButtonOk(true);
		    break;
		}
	    }
	}
    }
}

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

void VGCreateDialog::selectAll()
{
    for(int x = 0; x < m_pv_checks.size(); x++)
        m_pv_checks[x]->setChecked(true);
}

void VGCreateDialog::selectNone()
{
    for(int x = 0; x < m_pv_checks.size(); x++)
        m_pv_checks[x]->setChecked(false);
}