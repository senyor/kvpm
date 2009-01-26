/*
 *
 * 
 * Copyright (C) 2009 Benjamin Scott   <benscott@nwlink.com>
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

#include <KMessageBox>
#include <KLocale>
#include <QtGui>

#include "tablecreate.h"


bool create_table(QString devicePath)
{

    QString warning_message = i18n("By writing a new partition table to this device, "
                                   "any existing data on it will be lost!");

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


TableCreateDialog::TableCreateDialog(QString devicePath, QWidget *parent) : 
    KDialog(parent),
    m_device_path(devicePath)
{

    setWindowTitle( i18n("Create partition table") );

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    layout->addWidget( new QLabel( i18n("Create partition table on:") ) );
    QLabel *device_label = new QLabel("<b>" + m_device_path + "</b>");
    device_label->setAlignment(Qt::AlignHCenter);
    layout->addWidget( device_label );

    QGroupBox *radio_box = new QGroupBox("Table types");
    QVBoxLayout *radio_box_layout = new QVBoxLayout();
    radio_box->setLayout(radio_box_layout);
    layout->addWidget(radio_box);

    m_msdos_button = new QRadioButton("MS-DOS");
    m_gpt_button   = new QRadioButton("GPT");
    m_msdos_button->setChecked(true);
    radio_box_layout->addWidget( m_msdos_button );
    radio_box_layout->addWidget( m_gpt_button );

    connect(this, SIGNAL(okClicked()),
            this, SLOT(commitTable()));
}

void TableCreateDialog::commitTable()
{

    PedDevice   *ped_device    = ped_device_get ( m_device_path.toAscii().data() ); 
    PedDiskType *ped_disk_type = NULL;

    if( m_msdos_button->isChecked() )
        ped_disk_type = ped_disk_type_get( "msdos" );
    else
        ped_disk_type = ped_disk_type_get( "gpt" );

    PedDisk *ped_disk = ped_disk_new_fresh (ped_device, ped_disk_type);

    ped_disk_commit(ped_disk);

}
