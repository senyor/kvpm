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


#include <KLocale>
#include <QtGui>

#include "partitionproperties.h"
#include "physvol.h"
#include "storagepartition.h"


PartitionProperties::PartitionProperties( StoragePartition *Partition, QWidget *parent)
    : QWidget(parent) 
{
    QStringList mount_points;
    QList<int> mount_position;
    PhysVol *pv;
    
    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    QString path = Partition->getPartitionPath();

    if( Partition->isPV() ){
	pv = Partition->getPhysicalVolume();
	layout->addWidget( new QLabel( i18n("Physical Volume: %1").arg(path) ) );
	layout->addWidget( new QLabel( i18n("UUID: %1").arg( pv->getUuid() ) ) );
    }
    else
	layout->addWidget( new QLabel( i18n("Partition: %1").arg(path) ) );

    mount_points   = Partition->getMountPoints();
    mount_position = Partition->getMountPosition();

    for(int x = 0; x < mount_points.size(); x++){
        if( mount_position[x] > 1 )
	    mount_points[x] = mount_points[x] + QString("<%1>").arg(mount_position[x]);
        layout->addWidget( new QLabel( i18n("Mount point: %1").arg(mount_points[x]) ) );
    }
}

