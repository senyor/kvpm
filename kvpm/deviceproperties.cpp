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


#include "deviceproperties.h"


#include <KConfigSkeleton>
#include <KGlobal>
#include <KLocale>
#include <KSeparator>

#include <QtGui>

#include "misc.h"
#include "physvol.h"
#include "storagedevice.h"
#include "storagepartition.h"


DeviceProperties::DeviceProperties( StorageDevice *device, QWidget *parent) : QWidget(parent) 
{
    QStringList mount_points;
    QList<int>  mount_position;
    QStringList uuid;
    QLabel     *temp_label;
    PhysVol *pv;

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);

    QFrame *basic_info_frame = new QFrame;
    QVBoxLayout *basic_info_layout = new QVBoxLayout();
    basic_info_frame->setLayout(basic_info_layout);
    basic_info_frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    basic_info_frame->setLineWidth(2);   

    temp_label = new QLabel( QString("<b>%1</b>").arg( device->getName() ) );
    temp_label->setAlignment( Qt::AlignCenter );
    basic_info_layout->addWidget( temp_label );

    if( device->isPhysicalVolume() )
        basic_info_layout->addWidget( new QLabel( device->getDiskLabel() ) );
    else
        basic_info_layout->addWidget( new QLabel( i18n("Partition table: %1", device->getDiskLabel() ) ) );

    basic_info_layout->addWidget( new QLabel( i18n("Logical sector size: %1", device->getSectorSize() ) ) );
    basic_info_layout->addWidget( new QLabel( i18n("Physical sector size: %1", device->getPhysicalSectorSize() ) ) );
    basic_info_layout->addWidget( new QLabel( i18n("Sectors: %1", device->getSize() / device->getSectorSize() ) ) );

    if( !device->isWritable() )
        basic_info_layout->addWidget( new QLabel( i18nc("May be read and not written", "Read only") ) );
    else
        basic_info_layout->addWidget( new QLabel( i18n("Read/write") ) );

    if( device->isBusy() )
        basic_info_layout->addWidget( new QLabel( i18n("Busy: Yes") ) );
    else
        basic_info_layout->addWidget( new QLabel( i18n("Busy: No") ) );

    layout->addWidget(basic_info_frame);

    QFrame *hardware_info_frame = new QFrame;
    QVBoxLayout *hardware_info_layout = new QVBoxLayout();
    hardware_info_frame->setLayout(hardware_info_layout);

    hardware_info_frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    hardware_info_frame->setLineWidth(2);   

    temp_label = new QLabel( i18n("<b>Hardware</b>") );
    temp_label->setAlignment( Qt::AlignCenter );
    hardware_info_layout->addWidget( temp_label );
    temp_label = new QLabel( device->getHardware() );
    temp_label->setWordWrap(true);
    hardware_info_layout->addWidget( temp_label );

    layout->addWidget(hardware_info_frame);

    if( device->isPhysicalVolume() ){
        pv = device->getPhysicalVolume();

        QFrame *pv_info_frame = new QFrame;
        QVBoxLayout *pv_info_layout = new QVBoxLayout();
        pv_info_frame->setLayout(pv_info_layout);
        pv_info_frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
        pv_info_frame->setLineWidth(2);
   
	temp_label =  new QLabel( i18n("<b>Physical volume</b>") );
	temp_label->setAlignment( Qt::AlignCenter );
	pv_info_layout->addWidget( temp_label );

        if( pv->isActive() )
            temp_label = new QLabel( i18n("State: active") );
        else
            temp_label = new QLabel( i18n("State: inactive") );
	pv_info_layout->addWidget( temp_label );

	temp_label =  new QLabel( i18n("<b>UUID</b>") );
	temp_label->setAlignment( Qt::AlignCenter );
	pv_info_layout->addWidget( temp_label );

	uuid = splitUuid( pv->getUuid() );
	pv_info_layout->addWidget( new QLabel(uuid[0]) );
	pv_info_layout->addWidget( new QLabel(uuid[1]) );

	layout->addWidget(pv_info_frame);
    }

    layout->addStretch();
}

DeviceProperties::DeviceProperties( StoragePartition *partition, QWidget *parent)
    : QWidget(parent) 
{
    QStringList mount_points;
    QList<int>  mount_position;
    QStringList uuid;
    PhysVol *pv;
    QLabel *temp_label;

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);
    layout->setSpacing(0);
    layout->setMargin(0);

    QString path = partition->getName();

    QFrame *basic_info_frame = new QFrame;
    QVBoxLayout *basic_info_layout = new QVBoxLayout();
    basic_info_frame->setLayout(basic_info_layout);
    layout->addWidget(basic_info_frame);
    basic_info_frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    basic_info_frame->setLineWidth(2);   

    temp_label =  new QLabel( QString("<b>%1</b>").arg(path) );
    temp_label->setAlignment( Qt::AlignCenter );
    basic_info_layout->addWidget( temp_label );

    basic_info_layout->addWidget( new QLabel( i18n("First sector: %1", partition->getFirstSector() ) ) );
    basic_info_layout->addWidget( new QLabel( i18n("Last sector: %1", partition->getLastSector() ) ) );

    QFrame *flag_info_frame = new QFrame;
    QVBoxLayout *flag_info_layout = new QVBoxLayout();
    flag_info_frame->setLayout(flag_info_layout);
    flag_info_frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    flag_info_frame->setLineWidth(2);   

    QStringList flags;
    if( partition->getType() == "logical" || partition->getType() == "normal"){
        temp_label = new QLabel( i18n("<b>Flags</b>") );
	temp_label->setAlignment( Qt::AlignCenter );
	flag_info_layout->addWidget(temp_label);
	flags = partition->getFlags();

	if( flags.size() == 1)
	    if( flags[0].isEmpty() )
	        flags << "No flags set";

	for( int x = 0; x < flags.size(); x++){
	    temp_label = new QLabel( flags[x] );
	    flag_info_layout->addWidget(temp_label);
	}

	layout->addWidget(flag_info_frame);
    }

    KConfigSkeleton skeleton;

    bool show_mount, 
         show_fsuuid, 
         show_fslabel;

    skeleton.setCurrentGroup("DeviceProperties");
    skeleton.addItemBool("mount",   show_mount,   true);
    skeleton.addItemBool("fsuuid",  show_fsuuid,  false);
    skeleton.addItemBool("fslabel", show_fslabel, false);
    
    if( !partition->isPhysicalVolume() && ( partition->getType() != "extended" ) && !partition->getType().contains("freespace") ){
        QFrame *mount_info_frame = new QFrame;
        QVBoxLayout *mount_info_layout = new QVBoxLayout();
        mount_info_frame->setLayout(mount_info_layout);
        mount_info_frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
        mount_info_frame->setLineWidth(2);   

	mount_points   = partition->getMountPoints();
	mount_position = partition->getMountPosition();

	if( mount_points.size() <= 1 )
	    temp_label = new QLabel( i18n("<b>Mount point</b>") );
	else
	    temp_label = new QLabel( i18n("<b>Mount points</b>") );

	temp_label->setAlignment( Qt::AlignCenter );
	mount_info_layout->addWidget( temp_label );

	if( mount_points.size() ){
	    for(int x = 0; x < mount_points.size(); x++){
	        if( mount_position[x] > 1 )
		    mount_points[x] = mount_points[x] + QString("<%1>").arg(mount_position[x]);

		mount_info_layout->addWidget( new QLabel( mount_points[x] ) );
	    }
	}
	else{
	    mount_info_layout->addWidget( new QLabel( i18n("Not mounted") ) );
	}

        if(show_mount)
            layout->addWidget(mount_info_frame);

        QFrame *label_frame = new QFrame;
        QVBoxLayout *label_layout = new QVBoxLayout();
        label_frame->setLayout(label_layout);
        label_frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
        label_frame->setLineWidth(2);   

        if(show_fslabel){
            temp_label = new QLabel( i18n("<b>Filesystem LABEL</b>") );
            temp_label->setAlignment( Qt::AlignCenter );
            label_layout->addWidget(temp_label);
            
            temp_label = new QLabel( partition->getFilesystemLabel() );
            label_layout->addWidget(temp_label);
        }

        if(show_fsuuid){
            temp_label = new QLabel( i18n("<b>Filesystem UUID</b>") );
            temp_label->setAlignment( Qt::AlignCenter );
            label_layout->addWidget(temp_label);
            
            uuid = splitUuid( partition->getFilesystemUuid() );
            label_layout->addWidget( new QLabel(uuid[0]) );
            label_layout->addWidget( new QLabel(uuid[1]) );
        }

        if(show_fsuuid || show_fslabel)
            layout->addWidget(label_frame);
    }
    else if( partition->isPhysicalVolume() ){
        pv = partition->getPhysicalVolume();
        QFrame *pv_info_frame = new QFrame;
        QVBoxLayout *pv_info_layout = new QVBoxLayout();
        pv_info_frame->setLayout(pv_info_layout);
        pv_info_frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
        pv_info_frame->setLineWidth(2);   

	temp_label =  new QLabel( i18n("<b>Physical volume</b>") );
	temp_label->setAlignment( Qt::AlignCenter );
	pv_info_layout->addWidget( temp_label );

        if( pv->isActive() )
            temp_label = new QLabel( i18n("State: active") );
        else
            temp_label = new QLabel( i18n("State: inactive") );

	pv_info_layout->addWidget( temp_label );

	temp_label =  new QLabel( i18n("<b>UUID</b>") );
	temp_label->setAlignment( Qt::AlignCenter );
	pv_info_layout->addWidget( temp_label );

	uuid = splitUuid( pv->getUuid() );
	pv_info_layout->addWidget( new QLabel(uuid[0]) );
	pv_info_layout->addWidget( new QLabel(uuid[1]) );

	layout->addWidget(pv_info_frame);
    }

    layout->addStretch();
}

