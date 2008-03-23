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
    int segment_count = logicalVolume->getSegmentCount();
    
    QStringList pv_list;
    QLabel *temp_label;
    
    QVBoxLayout *layout = new QVBoxLayout();


    if((segment >= 0) && (segment_count > 1)){

	temp_label = new QLabel( "<b>" + 
				 logicalVolume->getName() + 
				 QString("[%1]").arg(segment) +
				 "</b>"  );

	temp_label->setAlignment(Qt::AlignCenter);
	layout->addWidget(temp_label);

	extents = logicalVolume->getSegmentExtents(segment);
	stripes = logicalVolume->getSegmentStripes(segment);
	stripe_size = logicalVolume->getSegmentStripeSize(segment);

	if( !logicalVolume->isMirror() ){
	    layout->addWidget(new QLabel( QString("Extents: %1").arg( extents ) ));

	    if( stripes != 1 ){
		layout->addWidget(new QLabel(QString("Stripes: %1").arg(stripes)));
		layout->addWidget(new QLabel(QString("Stripe size: %1").arg(stripe_size)));
	    }
	    else{
		layout->addWidget(new QLabel(QString("Stripes: none")));
		layout->addWidget(new QLabel(QString("Stripe size: n/a")));
	    }

	}
    }
    else if((segment >= 0) && (segment_count == 1)){
	temp_label = new QLabel( "<b> " + logicalVolume->getName() + "</b>" );
	temp_label->setAlignment(Qt::AlignCenter);
	layout->addWidget(temp_label);
	extents = logicalVolume->getSegmentExtents(segment);
	stripes = logicalVolume->getSegmentStripes(segment);
	stripe_size = logicalVolume->getSegmentStripeSize(segment);
	if( !logicalVolume->isMirror() ){
	    layout->addWidget(new QLabel( QString("Extents: %1").arg( extents ) ));

	    if( stripes != 1 ){
		layout->addWidget(new QLabel(QString("Stripes: %1").arg(stripes)));
		layout->addWidget(new QLabel(QString("Stripe size: %1").arg(stripe_size)));
	    }
	    else{
		layout->addWidget(new QLabel(QString("Stripes: none")));
		layout->addWidget(new QLabel(QString("Stripe size: n/a")));
	    }

	}
    }
    else{
	temp_label = new QLabel( "<b> " + logicalVolume->getName() + "</b>" );
	temp_label->setAlignment(Qt::AlignCenter);
	layout->addWidget(temp_label);
	extents = logicalVolume->getExtents();
	stripes = logicalVolume->getSegmentStripes(0);
	stripe_size = logicalVolume->getSegmentStripeSize(0);

	layout->addWidget(new QLabel( QString("Extents: %1").arg( extents ) ));

	if( !logicalVolume->isMirror() ){

	    if( stripes != 1 ){
		layout->addWidget(new QLabel(QString("Stripes: %1").arg(stripes)));
		layout->addWidget(new QLabel(QString("Stripe size: %1").arg(stripe_size)));
	    }
	    else{
		layout->addWidget(new QLabel(QString("Stripes: none")));
		layout->addWidget(new QLabel(QString("Stripe size: n/a")));
	    }

	}
    }

    layout->addWidget(new QLabel("Allocation policy: " + logicalVolume->getPolicy()));
    
    QStringList mount_points = logicalVolume->getMountPoints();

    if(mount_points.size() > 1){
	temp_label = new QLabel( "<b>Mount points</b>" ) ;
	temp_label->setAlignment(Qt::AlignCenter);
	layout->addWidget(temp_label);
    }
    else{
	temp_label = new QLabel( "<b>Mount point</b>" ) ;
	temp_label->setAlignment(Qt::AlignCenter);
	layout->addWidget(temp_label);
    }

    if(mount_points.size() == 0){
	temp_label = new QLabel( "<none>" ) ;
	temp_label->setAlignment(Qt::AlignLeft);
	layout->addWidget(temp_label);
    }
    else{
	for(int x = 0; x < mount_points.size(); x++){
	    temp_label = new QLabel( mount_points[x] );
	    temp_label->setToolTip( mount_points[x] );
	    layout->addWidget( temp_label );
	}
    }
    

    if(logicalVolume->isSnap())
	layout->addWidget(new QLabel("Snapshot origin: "  + logicalVolume->getOrigin()));

    if( logicalVolume->isMirror() ){
	temp_label = new QLabel("<b>Mirror legs</b>");
	temp_label->setAlignment(Qt::AlignCenter);
	layout->addWidget(temp_label);
    }
    else{
	temp_label = new QLabel("<b>Physical Volumes</b>");
	temp_label->setAlignment(Qt::AlignCenter);
	layout->addWidget(temp_label);
    }
    
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

    layout->addStretch();
    
    setLayout(layout);
}
