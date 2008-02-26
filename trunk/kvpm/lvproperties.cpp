/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <QtGui>
#include "logvol.h"
#include "lvproperties.h"

LVProperties::LVProperties(LogVol *logicalVolume, int segment, QWidget *parent):QWidget(parent)
{
    long long extents;
    
    QStringList pv_list;

    QVBoxLayout *layout = new QVBoxLayout();

    layout->addWidget(new QLabel( "<b>" + logicalVolume->getName() + "</b>" ), 
		      0, 
		      Qt::AlignHCenter);
     
    if(segment > -1){
	extents = logicalVolume->getSegmentExtents(segment);
	layout->addWidget(new QLabel(QString("Segment: %1").arg(segment)));
    }
    else{
	extents = logicalVolume->getExtents();
	layout->addWidget(new QLabel(QString("Segment: all")));
    }
    
    layout->addWidget(new QLabel("Allocation: " + logicalVolume->getPolicy()));
    layout->addWidget(new QLabel( QString("Extents: %1").arg( extents ) ));

    if(logicalVolume->isMounted()){
	QStringList mount_points = logicalVolume->getMountPoints();
	for(int x = 0; x < mount_points.size(); x++)
	    layout->addWidget( new QLabel( "Mount point: " + mount_points[x] ) );
    }
    else
	layout->addWidget(new QLabel("Mount point: Not mounted"));

    if(logicalVolume->isSnap())
	layout->addWidget(new QLabel("Snapshot origin: "  + logicalVolume->getOrigin()));

    layout->addStretch();

    layout->addWidget(new QLabel("Physical Volumes"), 0, Qt::AlignHCenter);

    if(segment > -1){
	pv_list = logicalVolume->getDevicePath(segment);
	for(int pv = 0; pv < pv_list.size(); pv++)
	    layout->addWidget(new QLabel(pv_list[pv]));
    }
    else{
	pv_list = logicalVolume->getDevicePathAll();
	for(int pv = 0; pv < pv_list.size(); pv++)
	    layout->addWidget(new QLabel(pv_list[pv]));
    }
    
    setLayout(layout);
}
