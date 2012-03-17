/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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

#include <KConfigSkeleton>
#include <KGlobal>
#include <KLocale>
#include <KMessageBox>
#include <KPushButton>

#include <QtGui>

#include "masterlist.h"
#include "misc.h"
#include "progressbox.h"
#include "pvgroupbox.h"
#include "storagedevice.h"
#include "storagepartition.h"
#include "topwindow.h"
#include "vgcreate.h"
#include "volgroup.h"



VGExtendDialog::VGExtendDialog(VolGroup *const group, QWidget *parent) 
    : KDialog(parent), m_vg(group)
{
    m_bailout = false;

    QList<StorageDevice *> devices;
    QList<StoragePartition *> partitions;
    const QString warning = i18n("If a device or partition is added to a volume group, "
                                 "any data currently on that device or partition will be lost.");

    getUsablePvs(devices, partitions); 


    if( partitions.size() + devices.size() > 0 ){
        if(KMessageBox::warningContinueCancel(NULL, 
                                              warning, 
                                              QString(), 
                                              KStandardGuiItem::cont(), 
                                              KStandardGuiItem::cancel(), 
                                              QString(), 
                                              KMessageBox::Dangerous) == KMessageBox::Continue){
            buildDialog(devices, partitions);
        }
        else{
            m_bailout = true;
        }
    }
    else{
        m_bailout = true;
        KMessageBox::error(0, i18n("No unused potential physical volumes found") );
    }
}

VGExtendDialog::VGExtendDialog(VolGroup *const group, StorageDevice *const device, StoragePartition *const partition, QWidget *parent) 
    : KDialog(parent), m_vg(group) 
{
    m_bailout = false;

    QList<StorageDevice *> devices;
    QList<StoragePartition *> partitions;

    if(device != NULL)
        devices.append(device);
    else
        partitions.append(partition);

    QString warning = i18n("If a device or partition is added to a volume group, "
                           "any data currently on that device or partition will be lost.");


    if(KMessageBox::warningContinueCancel(NULL, 
                                          warning, 
                                          QString(), 
                                          KStandardGuiItem::cont(), 
                                          KStandardGuiItem::cancel(), 
                                          QString(), 
                                          KMessageBox::Dangerous) == KMessageBox::Continue){
        buildDialog(devices, partitions);
    }
    else{
        m_bailout = true;
    }    
}

void VGExtendDialog::commitChanges()
{
    const QByteArray vg_name   = m_vg->getName().toLocal8Bit();
    const QStringList pv_names = m_pv_checkbox->getNames();
    QByteArray pv_name;
    lvm_t lvm = MasterList::getLvm();
    vg_t  vg_dm;

    ProgressBox *const progress_box = TopWindow::getProgressBox();
    progress_box->setRange(0, pv_names.size());
    progress_box->setText("Extending VG");

    hide();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    if( (vg_dm = lvm_vg_open(lvm, vg_name.data(), "w", 0 )) ){
        
        for(int x = 0; x < pv_names.size(); x++){
            progress_box->setValue(x);
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            pv_name = pv_names[x].toLocal8Bit();
            if( lvm_vg_extend(vg_dm, pv_name.data()) )
                KMessageBox::error(0, QString(lvm_errmsg(lvm)));
        }
        
        if( lvm_vg_write(vg_dm) )
            KMessageBox::error(0, QString(lvm_errmsg(lvm)));

        lvm_vg_close(vg_dm);
        progress_box->reset();
        return;
    }

    KMessageBox::error(0, QString(lvm_errmsg(lvm))); 
    progress_box->reset();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    return;
}

void VGExtendDialog::validateOK()
{
    if( m_pv_checkbox->getRemainingSpace() )
        enableButtonOk(true);
    else
        enableButtonOk(false);
}

void VGExtendDialog::buildDialog(QList<StorageDevice *> devices, QList<StoragePartition *> partitions)
{
    setWindowTitle( i18n("Extend Volume Group") );

    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    QLabel *const title = new QLabel( i18n("Extend Volume Group: <b>%1</b>", m_vg->getName()) );
    title->setAlignment(Qt::AlignCenter);
    layout->addSpacing(5);
    layout->addWidget(title);
    layout->addSpacing(5);

    m_pv_checkbox = new PvGroupBox( devices, partitions, m_vg->getExtentSize() );
    layout->addWidget(m_pv_checkbox);

    connect(m_pv_checkbox, SIGNAL(stateChanged()), 
	    this, SLOT(validateOK()));

    connect(this, SIGNAL(okClicked()), 
	    this, SLOT(commitChanges()));
}

void VGExtendDialog::getUsablePvs(QList<StorageDevice *> &devices, QList<StoragePartition *> &partitions) 
{
    QList<StorageDevice *> all_dev = MasterList::getStorageDevices();   
    QList<StoragePartition *> all_part;

    for(int x = 0; x < all_dev.size(); x++){
        if( (all_dev[x]->getRealPartitionCount() == 0) && 
            ( !all_dev[x]->isBusy()) && 
            ( !all_dev[x]->isPhysicalVolume() )) {

            devices.append( all_dev[x] );
        }
        else if( all_dev[x]->getRealPartitionCount() > 0 ){
            all_part = all_dev[x]->getStoragePartitions();
            for(int y = 0; y < all_part.size(); y++){
                if( ( !all_part[y]->isBusy() ) && (! all_part[y]->isPhysicalVolume() ) &&
                    (( all_part[y]->isNormal() ) || ( all_part[y]->isLogical() )))  {

                    partitions.append(all_part[y]); 
                }
            }
        }
    }
}

bool VGExtendDialog::bailout()
{
    return m_bailout;
}
