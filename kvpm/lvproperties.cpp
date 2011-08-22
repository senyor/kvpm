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

#include <KLocale>
#include <QtGui>

#include "logvol.h"
#include "lvproperties.h"
#include "misc.h"
#include "volgroup.h"


/* if segment = -1 we have a multi segement logical volume but
   we are not focused on any one segment. Therefor stripes and
   stripe size have no meaning */

LVProperties::LVProperties(LogVol *logicalVolume, int segment, QWidget *parent):QWidget(parent)
{
    const long long extent_size = logicalVolume->getVolumeGroup()->getExtentSize();
    const int segment_count = logicalVolume->getSegmentCount();
    long long extents, total_size, total_extents;
    int stripes, stripe_size;
    QStringList pv_list;
    QLabel *temp_label;
    
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setMargin(0);

    QFrame *basic_info_frame = new QFrame();
    QVBoxLayout *basic_info_layout = new QVBoxLayout();
    basic_info_frame->setLayout(basic_info_layout);
    basic_info_frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    basic_info_frame->setLineWidth(2);
    layout->addWidget(basic_info_frame);

    if((segment >= 0) && (segment_count > 1)){

	temp_label = new QLabel( "<b>" + logicalVolume->getName() + QString("[%1]").arg(segment) + "</b>" );
	temp_label->setAlignment(Qt::AlignCenter);
	basic_info_layout->addWidget(temp_label);

	extents = logicalVolume->getSegmentExtents(segment);
	stripes = logicalVolume->getSegmentStripes(segment);
	stripe_size = logicalVolume->getSegmentStripeSize(segment);

	basic_info_layout->addWidget(new QLabel( i18n("Extents: %1", extents) ) );

	if( !logicalVolume->isMirror() ){

	    if( stripes != 1 ){
		basic_info_layout->addWidget(new QLabel( i18n("Stripes: %1", stripes) ));
                basic_info_layout->addWidget(new QLabel( i18n("Stripe size: %1", stripe_size) ));
	    }
	    else{
		basic_info_layout->addWidget(new QLabel( i18n("Stripes: none") ));
		basic_info_layout->addWidget(new QLabel( i18n("Stripe size: n/a") ));
	    }
	}
    }
    else if((segment >= 0) && (segment_count == 1)){

	temp_label = new QLabel( "<b> " + logicalVolume->getName() + "</b>" );
	temp_label->setAlignment(Qt::AlignCenter);
	basic_info_layout->addWidget(temp_label);
	extents = logicalVolume->getSegmentExtents(segment);
        total_extents = (extents * logicalVolume->getMirrorCount()) + logicalVolume->getLogCount();
        total_size = extent_size * total_extents;
	stripes = logicalVolume->getSegmentStripes(segment);
	stripe_size = logicalVolume->getSegmentStripeSize(segment);
	basic_info_layout->addWidget(new QLabel( i18n("Extents: %1", extents) ));

	if( !logicalVolume->isMirror() ){

	    if( stripes != 1 ){
		basic_info_layout->addWidget(new QLabel( i18n("Stripes: %1", stripes) ));
                basic_info_layout->addWidget(new QLabel( i18n("Stripe size: %1", stripe_size) ));
	    }
	    else{
		basic_info_layout->addWidget(new QLabel( i18n("Stripes: none") ));
		basic_info_layout->addWidget(new QLabel( i18n("Stripe size: n/a") ));
	    }
	}
	else if( !logicalVolume->isMirrorLog() ){
            basic_info_layout->addWidget(new QLabel( i18n("Total extents: %1", total_extents) ));
	    basic_info_layout->addWidget(new QLabel( i18n("Total size: %1", sizeToString(total_size)) ));
	}
	if( !(logicalVolume->isMirrorLeg() || logicalVolume->isMirrorLog() )){

            basic_info_layout->addWidget(new QLabel( i18n("Filesystem: %1", logicalVolume->getFilesystem() ) ));

	    if(logicalVolume->isWritable())
	        basic_info_layout->addWidget(new QLabel( i18n("Access: r/w") ));
	    else
	        basic_info_layout->addWidget(new QLabel( i18n("Access: r/o") ));
                                         
            basic_info_layout->addWidget(new QLabel( i18n("Allocation policy: %1", logicalVolume->getPolicy() ) ));
	}
    }
    else{
	temp_label = new QLabel( "<b> " + logicalVolume->getName() + "</b>" );
	temp_label->setAlignment(Qt::AlignCenter);
	basic_info_layout->addWidget(temp_label);
	extents = logicalVolume->getExtents();
	basic_info_layout->addWidget(new QLabel( i18n("Extents: %1", extents) ));

	if( !( logicalVolume->isMirrorLeg() || logicalVolume->isMirrorLog() )){

            basic_info_layout->addWidget(new QLabel( i18n("Filesystem %1", logicalVolume->getFilesystem()) ));

	    if(logicalVolume->isWritable())
	        basic_info_layout->addWidget(new QLabel( i18n("Access: r/w") ));
	    else
	        basic_info_layout->addWidget(new QLabel( i18n("Access: r/o") ));

	    basic_info_layout->addWidget(new QLabel( i18n("Allocation policy: %1", logicalVolume->getPolicy())));
	}
    }

    if(logicalVolume->isSnap())
        basic_info_layout->addWidget(new QLabel( i18n("Origin: %1", logicalVolume->getOrigin()) ));
    
    QFrame *mount_info_frame = new QFrame();
    QVBoxLayout *mount_info_layout = new QVBoxLayout();
    mount_info_frame->setLayout(mount_info_layout);
    mount_info_frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    mount_info_frame->setLineWidth(2);

    QStringList mount_points = logicalVolume->getMountPoints();
    QList<int>  mount_position = logicalVolume->getMountPosition();

    if( !logicalVolume->isMirrorLeg() && !logicalVolume->isMirrorLog() &&
	!logicalVolume->isPvmove()    && !logicalVolume->isVirtual() &&
	( (segment_count == 1) ||
	  (segment == -1) ) )
    {
        layout->addWidget(mount_info_frame); // don't add this widget at all if in a mirror leg 

	if(mount_points.size() > 1){
            temp_label = new QLabel( i18n("<b>Mount points</b>") );
	    temp_label->setAlignment(Qt::AlignCenter);
	    mount_info_layout->addWidget(temp_label);
	}
	else{
	    temp_label = new QLabel( i18n("<b>Mount point</b>") ) ;
	    temp_label->setAlignment(Qt::AlignCenter);
	    mount_info_layout->addWidget(temp_label);
	}
	
	if(mount_points.size() == 0){
	    temp_label = new QLabel( i18n("not mounted") ) ;
	    temp_label->setAlignment(Qt::AlignLeft);
	    mount_info_layout->addWidget(temp_label);
	}
	else{

	    for(int x = 0; x < mount_points.size(); x++){
	        if(mount_position[x] > 0){
		    temp_label = new QLabel( QString("%1 <%2>").arg(mount_points[x]).arg(mount_position[x]) );
		    temp_label->setToolTip( QString("%1 <%2>").arg(mount_points[x]).arg(mount_position[x]) );
		    mount_info_layout->addWidget( temp_label );
		}
		else{
		    temp_label = new QLabel( mount_points[x] );
		    temp_label->setToolTip( mount_points[x] );
		    mount_info_layout->addWidget( temp_label );
		}
	    }
	}
    }

    QFrame *pv_info_frame = new QFrame();
    QVBoxLayout *pv_info_layout = new QVBoxLayout();
    pv_info_frame->setLayout(pv_info_layout);
    pv_info_frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    pv_info_frame->setLineWidth(2);
    layout->addWidget(pv_info_frame);

    if( logicalVolume->isMirror() ){
        temp_label = new QLabel( i18n("<b>Mirror legs</b>") );
	temp_label->setAlignment(Qt::AlignCenter);
	pv_info_layout->addWidget(temp_label);
    }
    else{
        temp_label = new QLabel( i18n("<b>Physical volumes</b>") );
	temp_label->setAlignment(Qt::AlignCenter);
	pv_info_layout->addWidget(temp_label);
    }
    
    if(segment > -1){
	pv_list = logicalVolume->getDevicePath(segment);
	for(int pv = 0; pv < pv_list.size(); pv++){
	    temp_label = new QLabel( pv_list[pv] );
	    temp_label->setToolTip( pv_list[pv] );
	    pv_info_layout->addWidget( temp_label );
	}
    }
    else{
	pv_list = logicalVolume->getDevicePathAll();
	for(int pv = 0; pv < pv_list.size(); pv++){
	    temp_label = new QLabel( pv_list[pv] );
	    temp_label->setToolTip( pv_list[pv] );
	    pv_info_layout->addWidget( temp_label );
	}

    }

    QFrame *uuid_info_frame = new QFrame();
    QVBoxLayout *uuid_info_layout = new QVBoxLayout();
    uuid_info_frame->setLayout(uuid_info_layout);
    uuid_info_frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    uuid_info_frame->setLineWidth(2);
    layout->addWidget(uuid_info_frame);
    temp_label = new QLabel("<b>Logical volume UUID</b>");
    temp_label->setAlignment(Qt::AlignCenter);
    uuid_info_layout->addWidget( temp_label );
    temp_label = new QLabel( logicalVolume->getUuid() );
    temp_label->setToolTip( logicalVolume->getUuid() );
    temp_label->setWordWrap(true);

    uuid_info_layout->addWidget( temp_label );
    uuid_info_layout->addStretch();
    
    setLayout(layout);
}
