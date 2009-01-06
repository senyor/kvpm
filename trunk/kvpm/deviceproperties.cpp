/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
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
    KSeparator *separator;

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);
    
    temp_label = new QLabel( QString("<b>%1</b>").arg( Device->getDevicePath() ) );
    temp_label->setAlignment( Qt::AlignCenter );
    layout->addWidget( temp_label );

    layout->addWidget( new QLabel( i18n("Partition table: %1").arg( Device->getDiskLabel() ) ) );
    layout->addWidget( new QLabel( i18n("Logical sector size: %1").arg( Device->getSectorSize() ) ) );
    layout->addWidget( new QLabel( i18n("Physical sector size: %1").arg( Device->getPhysicalSectorSize() ) ) );
 
    if( Device->isReadOnly() )
        layout->addWidget( new QLabel( i18n("Read only") ) );
    else
        layout->addWidget( new QLabel( i18n("Read/write") ) );

    separator = new KSeparator(Qt::Horizontal);
    separator->setFrameStyle(QFrame::Plain | QFrame::Box);
    separator->setLineWidth(2);
    separator->setMaximumHeight(2);
    layout->addWidget(separator);

    temp_label = new QLabel( i18n("<b>Hardware</b>") );
    temp_label->setAlignment( Qt::AlignCenter );
    layout->addWidget( temp_label );

    temp_label = new QLabel( Device->getHardware() );
    temp_label->setWordWrap(true);
    layout->addWidget( temp_label );


    layout->addStretch();

}

DeviceProperties::DeviceProperties( StoragePartition *Partition, QWidget *parent)
    : QWidget(parent) 
{
    QStringList mount_points;
    QList<int> mount_position;
    PhysVol *pv;
    QLabel *temp_label;

    KSeparator *separator;
    
    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    QString path = Partition->getPartitionPath();

    temp_label =  new QLabel( QString("<b>%1</b>").arg(path) );
    temp_label->setAlignment( Qt::AlignCenter );
    layout->addWidget( temp_label );

    layout->addWidget( new QLabel( i18n("First sector: %1", Partition->getFirstSector() ) ) );
    layout->addWidget( new QLabel( i18n("Last sector: %1", Partition->getLastSector() ) ) );

    if( Partition->isPV() ){
        pv = Partition->getPhysicalVolume();
 
        separator = new KSeparator(Qt::Horizontal);
        separator->setFrameStyle(QFrame::Plain | QFrame::Box);
        separator->setLineWidth(2);
        separator->setMaximumHeight(2);
        layout->addWidget(separator);

	temp_label =  new QLabel( "<b>Physical volume UUID</b>" );
	temp_label->setAlignment( Qt::AlignCenter );
	layout->addWidget( temp_label );

	temp_label =  new QLabel( pv->getUuid() );
	temp_label->setWordWrap(true);
	layout->addWidget( temp_label );
    }

    mount_points   = Partition->getMountPoints();
    mount_position = Partition->getMountPosition();

    for(int x = 0; x < mount_points.size(); x++){
        if( mount_position[x] > 1 )
	    mount_points[x] = mount_points[x] + QString("<%1>").arg(mount_position[x]);
        layout->addWidget( new QLabel( i18n("Mount point: %1").arg(mount_points[x]) ) );
    }

    layout->addStretch();

}

