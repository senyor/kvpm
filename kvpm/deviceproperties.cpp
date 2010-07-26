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


#include <QtGui>
#include <KLocale>
#include <KSeparator>

#include "physvol.h"
#include "deviceproperties.h"
#include "storagedevice.h"
#include "storagepartition.h"


DeviceProperties::DeviceProperties( StorageDevice *Device, QWidget *parent) : QWidget(parent) 
{

    QStringList mount_points;
    QList<int>  mount_position;
    QLabel     *temp_label;

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);

    QFrame *basic_info_frame = new QFrame;
    QVBoxLayout *basic_info_layout = new QVBoxLayout();
    basic_info_frame->setLayout(basic_info_layout);
    basic_info_frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    basic_info_frame->setLineWidth(2);   

    temp_label = new QLabel( QString("<b>%1</b>").arg( Device->getDevicePath() ) );
    temp_label->setAlignment( Qt::AlignCenter );
    basic_info_layout->addWidget( temp_label );

    basic_info_layout->addWidget( new QLabel( i18n("Partition table: %1").arg( Device->getDiskLabel() ) ) );
    basic_info_layout->addWidget( new QLabel( i18n("Logical sector size: %1").arg( Device->getSectorSize() ) ) );
    basic_info_layout->addWidget( new QLabel( i18n("Physical sector size: %1").arg( Device->getPhysicalSectorSize() ) ) );
 
    if( !Device->isWritable() )
        basic_info_layout->addWidget( new QLabel( i18n("Read only") ) );
    else
        basic_info_layout->addWidget( new QLabel( i18n("Read/write") ) );

    if( Device->isBusy() )
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

    temp_label = new QLabel( Device->getHardware() );
    temp_label->setWordWrap(true);
    hardware_info_layout->addWidget( temp_label );
    hardware_info_layout->addStretch();

    layout->addWidget(hardware_info_frame);

}

DeviceProperties::DeviceProperties( StoragePartition *Partition, QWidget *parent)
    : QWidget(parent) 
{
    QStringList mount_points;
    QList<int> mount_position;
    PhysVol *pv;
    QLabel *temp_label;

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);
    layout->setSpacing(0);
    layout->setMargin(0);

    QString path = Partition->getName();

    QFrame *basic_info_frame = new QFrame;
    QVBoxLayout *basic_info_layout = new QVBoxLayout();
    basic_info_frame->setLayout(basic_info_layout);
    layout->addWidget(basic_info_frame);
    basic_info_frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    basic_info_frame->setLineWidth(2);   

    temp_label =  new QLabel( QString("<b>%1</b>").arg(path) );
    temp_label->setAlignment( Qt::AlignCenter );
    basic_info_layout->addWidget( temp_label );

    basic_info_layout->addWidget( new QLabel( i18n("First sector: %1", Partition->getFirstSector() ) ) );
    basic_info_layout->addWidget( new QLabel( i18n("Last sector: %1", Partition->getLastSector() ) ) );

    QFrame *pv_info_frame = new QFrame;
    QVBoxLayout *pv_info_layout = new QVBoxLayout();
    pv_info_frame->setLayout(pv_info_layout);
    pv_info_frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    pv_info_frame->setLineWidth(2);   

    QFrame *mount_info_frame = new QFrame;
    QVBoxLayout *mount_info_layout = new QVBoxLayout();
    mount_info_frame->setLayout(mount_info_layout);
    mount_info_frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    mount_info_frame->setLineWidth(2);   

    if( Partition->isPV() ){

        pv = Partition->getPhysicalVolume();

	temp_label =  new QLabel( "<b>Physical volume</b>" );
	temp_label->setAlignment( Qt::AlignCenter );
	pv_info_layout->addWidget( temp_label );

        if( pv->isActive() )
            temp_label = new QLabel( "active" );
        else
            temp_label = new QLabel( "inactive" );

	pv_info_layout->addWidget( temp_label );

	temp_label =  new QLabel( "<b>UUID</b>" );
	temp_label->setAlignment( Qt::AlignCenter );
	pv_info_layout->addWidget( temp_label );

	temp_label =  new QLabel( pv->getUuid() );
	temp_label->setWordWrap(true);
	pv_info_layout->addWidget( temp_label );
	layout->addWidget(pv_info_frame);
    }
    else{

	mount_points   = Partition->getMountPoints();
	mount_position = Partition->getMountPosition();

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
        layout->addWidget(mount_info_frame);
    }

    QFrame *flag_info_frame = new QFrame;
    QVBoxLayout *flag_info_layout = new QVBoxLayout();
    flag_info_frame->setLayout(flag_info_layout);
    flag_info_frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    flag_info_frame->setLineWidth(2);   

    QStringList flags;
    if( Partition->getType() == "logical" || Partition->getType() == "normal"){
        temp_label = new QLabel( i18n("<b>Flags</b>") );
	temp_label->setAlignment( Qt::AlignCenter );
	flag_info_layout->addWidget(temp_label);
	flags = Partition->getFlags();

	if( flags.size() == 1)
	    if( flags[0] == "" )
	        flags << "No flags set";

	for( int x = 0; x < flags.size(); x++){
	    temp_label = new QLabel( flags[x] );
	    flag_info_layout->addWidget(temp_label);
	}

	layout->addWidget(flag_info_frame);
    }

    layout->addStretch();
}

