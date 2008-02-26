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


/* if segment = -1 we have a multi segement logical volume but
   we are not focused on any one segment. Therefor stripes and
   stripe size have no meaning */

LVProperties::LVProperties(LogVol *logicalVolume, int segment, QWidget *parent):QWidget(parent)
{
    long long extents;
    int stripes;
    int stripe_size;
    
    QStringList pv_list;
    QLabel *temp_label;
    
    QVBoxLayout *layout = new QVBoxLayout();

    layout->addWidget(new QLabel( "<b> " + logicalVolume->getName() + "</b>" ) );

    if(segment >= 0){
	extents = logicalVolume->getSegmentExtents(segment);
	stripes = logicalVolume->getSegmentStripes(segment);
	stripe_size = logicalVolume->getSegmentStripeSize(segment);
	if( !logicalVolume->isMirror() ){
	    layout->addWidget(new QLabel(QString("Segment: %1").arg(segment)));
	    layout->addWidget(new QLabel(QString("Stripes: %1").arg(stripes)));

	    if( stripes != 1 )
		layout->addWidget(new QLabel(QString("Stripe size: %1").arg(stripe_size)));
	}
    }
    else{
	extents = logicalVolume->getExtents();
	stripes = logicalVolume->getSegmentStripes(0);
	stripe_size = logicalVolume->getSegmentStripeSize(0);
	if( !logicalVolume->isMirror() ){
	    layout->addWidget(new QLabel(QString("Segment: all")));
	    layout->addWidget(new QLabel(QString("Stripes: %1").arg(stripes)));

	    if( stripes != 1 )
		layout->addWidget(new QLabel(QString("Stripe size: %1").arg(stripe_size)));
	}
    }
    
    layout->addWidget(new QLabel("Allocation policy: " + logicalVolume->getPolicy()));
    layout->addWidget(new QLabel( QString("Extents: %1").arg( extents ) ));

    layout->addStretch();

    if(logicalVolume->isMounted()){
	QStringList mount_points = logicalVolume->getMountPoints();

	if(mount_points.size() > 1)
	    layout->addWidget( new QLabel( "<b> Mount points</b>" ) );
	
	else
	    layout->addWidget( new QLabel( "<b> Mount point</b>" ) );
	
	for(int x = 0; x < mount_points.size(); x++){
	    temp_label = new QLabel( mount_points[x] );
	    temp_label->setToolTip( mount_points[x] );
	    layout->addWidget( temp_label );
	}
	
    }
    else{
	layout->addWidget(new QLabel("<b>Not mounted</b>") );
    }

    if(logicalVolume->isSnap())
	layout->addWidget(new QLabel("Snapshot origin: "  + logicalVolume->getOrigin()));

    layout->addStretch();

    if( logicalVolume->isMirror() )
	layout->addWidget(new QLabel("<b>Mirror legs</b>") );
    else
	layout->addWidget(new QLabel("<b>Physical Volumes</b>") );

    if(segment > -1){
	pv_list = logicalVolume->getDevicePath(segment);
	for(int pv = 0; pv < pv_list.size(); pv++){
	    temp_label = new QLabel( pv_list[pv] );
	    temp_label->setToolTip( pv_list[pv] );
	    layout->addWidget( temp_label );
	}
    }
    else{
	pv_list = logicalVolume->getDevicePathAll();
	for(int pv = 0; pv < pv_list.size(); pv++){
	    temp_label = new QLabel( pv_list[pv] );
	    temp_label->setToolTip( pv_list[pv] );
	    layout->addWidget( temp_label );
	}

    }
    
    setLayout(layout);
}
