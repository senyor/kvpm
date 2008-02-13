/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Klvm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <QtGui>

#include "partitionproperties.h"
#include "physvol.h"
#include "storagepartition.h"


PartitionProperties::PartitionProperties( StoragePartition *Partition, QWidget *parent)
    : QWidget(parent) 
{
    QStringList mount_points;
    PhysVol *pv;
    
    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    QString path = Partition->getPartitionPath();

    if( Partition->isPV() ){
	pv = Partition->getPhysicalVolume();
	layout->addWidget( new QLabel( QString("Physical Volume: %1").arg(path) ) );
	layout->addWidget( new QLabel( QString("UUID: %1").arg( pv->getUuid() ) ) );
    }
    else
	layout->addWidget( new QLabel( QString("Partition: %1").arg(path) ) );

    mount_points = Partition->getMountPoints();
    

    for(int x = 0; x < mount_points.size(); x++)
	layout->addWidget( new QLabel("Mount point: " + mount_points[x] ) );


}

