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

#include <KSeparator>
#include <KLocale>
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
    KSeparator *separator;
    
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

	layout->addWidget(new QLabel( i18n("Extents: %1").arg(extents) ));

	if( !logicalVolume->isMirror() ){

	    if( stripes != 1 ){
		layout->addWidget(new QLabel( i18n("Stripes: %1").arg(stripes) ));
		layout->addWidget(new QLabel( i18n("Stripe size: %1").arg(stripe_size) ));
	    }
	    else{
		layout->addWidget(new QLabel( i18n("Stripes: none") ));
		layout->addWidget(new QLabel( i18n("Stripe size: n/a") ));
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

	layout->addWidget(new QLabel( i18n("Extents: %1").arg(extents) ));

	if( !logicalVolume->isMirror() ){

	    if( stripes != 1 ){
		layout->addWidget(new QLabel( i18n("Stripes: %1").arg(stripes) ));
		layout->addWidget(new QLabel( i18n("Stripe size: %1").arg(stripe_size) ));
	    }
	    else{
		layout->addWidget(new QLabel( i18n("Stripes: none") ));
		layout->addWidget(new QLabel( i18n("Stripe size: n/a") ));
	    }

	}
	if( !(logicalVolume->isMirrorLeg() || logicalVolume->isMirrorLog() )){
	    layout->addWidget(new QLabel( i18n("Allocation policy: %1").arg(logicalVolume->getPolicy())));
	}
    }
    else{
	temp_label = new QLabel( "<b> " + logicalVolume->getName() + "</b>" );
	temp_label->setAlignment(Qt::AlignCenter);
	layout->addWidget(temp_label);

	extents = logicalVolume->getExtents();
	layout->addWidget(new QLabel( i18n("Extents: %1").arg(extents) ));
	if( !( logicalVolume->isMirrorLeg() || logicalVolume->isMirrorLog() )){
	    layout->addWidget(new QLabel( i18n("Allocation policy: %1").arg(logicalVolume->getPolicy())));
	}
    }

    if(logicalVolume->isSnap())
        layout->addWidget(new QLabel( i18n("Origin: %1").arg(logicalVolume->getOrigin()) ));
    
    QStringList mount_points = logicalVolume->getMountPoints();
    QList<int>  mount_position = logicalVolume->getMountPosition();

    if( !logicalVolume->isMirrorLeg() && 
	!logicalVolume->isMirrorLog() &&
	( (segment_count == 1) ||
	  (segment == -1) ) )
    {

	separator = new KSeparator(Qt::Horizontal);
	separator->setFrameStyle(QFrame::Plain | QFrame::Box);
	separator->setLineWidth(2);
	separator->setMaximumHeight(2);
	layout->addWidget(separator);

	if(mount_points.size() > 1){
	  temp_label = new QLabel( i18n("<b>Mount points</b>") );
	    temp_label->setAlignment(Qt::AlignCenter);
	    layout->addWidget(temp_label);
	}
	else{
	    temp_label = new QLabel( i18n("<b>Mount point</b>") ) ;
	    temp_label->setAlignment(Qt::AlignCenter);
	    layout->addWidget(temp_label);
	}
	
	if(mount_points.size() == 0){
	    temp_label = new QLabel( i18n("<none>") ) ;
	    temp_label->setAlignment(Qt::AlignLeft);
	    layout->addWidget(temp_label);
	}
	else{

	    for(int x = 0; x < mount_points.size(); x++){
	        if(mount_position[x] > 0){
		    temp_label = new QLabel( QString("%1 <%2>").arg(mount_points[x]).arg(mount_position[x]) );
		    temp_label->setToolTip( QString("%1 <%2>").arg(mount_points[x]).arg(mount_position[x]) );
		    layout->addWidget( temp_label );
		}
		else{
		    temp_label = new QLabel( mount_points[x] );
		    temp_label->setToolTip( mount_points[x] );
		    layout->addWidget( temp_label );
		}
	    }
	}
    }
    
    separator = new KSeparator(Qt::Horizontal);
    separator->setFrameStyle(QFrame::Plain | QFrame::Box);
    separator->setLineWidth(2);
    separator->setMaximumHeight(2);
    layout->addWidget(separator);

    if( logicalVolume->isMirror() ){
        temp_label = new QLabel( i18n("<b>Mirror legs</b>") );
	temp_label->setAlignment(Qt::AlignCenter);
	layout->addWidget(temp_label);
    }
    else{
        temp_label = new QLabel( i18n("<b>Physical Volumes</b>") );
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
