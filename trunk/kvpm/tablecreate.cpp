/*
 *
 * 
 * Copyright (C) 2009, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include "tablecreate.h"

#include <parted/parted.h>

#include <KMessageBox>
#include <KLocale>
#include <QtGui>



//  Creates or deletes a partition table "disk label" on a device.


bool create_table(const QString devicePath)
{

    const QString warning_message = i18n("Writing a new partition table to this device, " 
                                         "or removing the old one, will cause "
                                         "any existing data on it to be permanently lost");

    if(KMessageBox::warningContinueCancel(0, warning_message) != KMessageBox::Continue)
        return false;
    else{
        TableCreateDialog dialog(devicePath);
	dialog.exec();
    
	if(dialog.result() == QDialog::Accepted)
	    return true;
	else
	    return false;
    }
}


TableCreateDialog::TableCreateDialog(const QString devicePath, QWidget *parent) : 
    KDialog(parent),
    m_device_path(devicePath)
{
    setWindowTitle( i18n("Create Partition Table") );

    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    layout->addWidget( new QLabel( i18n("Create partition table on:") ) );
    QLabel *const device_label = new QLabel("<b>" + m_device_path + "</b>");
    device_label->setAlignment(Qt::AlignHCenter);
    layout->addWidget(device_label);

    QGroupBox *const radio_box = new QGroupBox("Table Types");
    QVBoxLayout *const radio_box_layout = new QVBoxLayout();
    radio_box->setLayout(radio_box_layout);
    layout->addWidget(radio_box);

    m_msdos_button   = new QRadioButton("MS-DOS");
    m_gpt_button     = new QRadioButton("GPT");
    m_destroy_button = new QRadioButton("Remove table");
    m_msdos_button->setChecked(true);
    radio_box_layout->addWidget(m_msdos_button);
    radio_box_layout->addWidget(m_gpt_button);
    radio_box_layout->addWidget(m_destroy_button);

    connect(this, SIGNAL(okClicked()),
            this, SLOT(commitTable()));

    setDefaultButton(KDialog::Cancel);
}

void TableCreateDialog::commitTable()
{
    QByteArray path = m_device_path.toLocal8Bit(); 
    PedDevice   *const ped_device = ped_device_get( path.data() ); 
    PedDiskType *ped_disk_type = NULL;
    PedDisk     *ped_disk = NULL;
    char        *buff = NULL;

    if( m_msdos_button->isChecked() ){
        ped_disk_type = ped_disk_type_get("msdos");
        ped_disk = ped_disk_new_fresh (ped_device, ped_disk_type);
        ped_disk_commit(ped_disk);
    }
    else if( m_gpt_button->isChecked() ){
        ped_disk_type = ped_disk_type_get("gpt");
        ped_disk = ped_disk_new_fresh (ped_device, ped_disk_type);
        ped_disk_commit(ped_disk);
    }
    else{
        ped_disk_clobber(ped_device); // This isn't enough for lvm
        ped_device_open(ped_device);
        buff = static_cast<char *>( malloc( 2 * ped_device->sector_size ) );

        for( int x = 0; x < 2 * ped_device->sector_size; x++)
            buff[x] = 0;

        if( ! ped_device_write(ped_device, buff, 0, 2) )  // clobber first 2 sectors
            KMessageBox::error( 0, "Destroying table failed: could not write to device");

        ped_device_close(ped_device);
    }
}


