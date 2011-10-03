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


#include "lvproperties.h"

#include <KLocale>
#include <QtGui>

#include "logvol.h"
#include "misc.h"
#include "volgroup.h"


/* if segment = -1 we have a multi segement logical volume but
   we are not focused on any one segment. Therefor stripes and
   stripe size have no meaning */

LVProperties::LVProperties(LogVol *logicalVolume, int segment, QWidget *parent):
    QWidget(parent),
    m_lv(logicalVolume)
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addWidget( generalFrame(segment) );

    if( !m_lv->isMirrorLeg() && !m_lv->isMirrorLog() &&
	!m_lv->isPvmove()    && !m_lv->isVirtual() &&
        !m_lv->isSnapContainer() && ( (m_lv->getSegmentCount() == 1) || (segment == -1) ) ){

        layout->addWidget( mountPointsFrame() );
        layout->addWidget( physicalVolumesFrame(segment) );
        layout->addWidget( fsFrame() );
    }
    else
        layout->addWidget( physicalVolumesFrame(segment) );

    if( !m_lv->isSnapContainer() && ( (m_lv->getSegmentCount() == 1) || (segment == -1) ) )
        layout->addWidget( uuidFrame() );

    layout->addStretch();
    
    setLayout(layout);
}

QFrame *LVProperties::mountPointsFrame()
{
    QLabel *temp_label;
    QFrame *frame = new QFrame();
    QVBoxLayout *layout = new QVBoxLayout();
    frame->setLayout(layout);
    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);

    QStringList mount_points   = m_lv->getMountPoints();
    QList<int>  mount_position = m_lv->getMountPosition();

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
        temp_label = new QLabel( i18n("not mounted") ) ;
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

    return frame;
}

QFrame *LVProperties::uuidFrame()
{
    QFrame *frame = new QFrame();
    QVBoxLayout *layout = new QVBoxLayout();
    frame->setLayout(layout);
    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);

    QLabel *temp_label = new QLabel( i18n("<b>Logical volume UUID</b>") );
    temp_label->setAlignment(Qt::AlignCenter);
    layout->addWidget( temp_label );
    temp_label = new QLabel( m_lv->getUuid() );
    temp_label->setToolTip( m_lv->getUuid() );
    temp_label->setWordWrap(true);
    layout->addWidget( temp_label );

    return frame;
}

QFrame *LVProperties::fsFrame()
{
    QFrame *frame = new QFrame();
    QVBoxLayout *layout = new QVBoxLayout();
    frame->setLayout(layout);
    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);

    QLabel *temp_label = new QLabel( i18n("<b>Filesystem label</b>") );
    temp_label->setAlignment(Qt::AlignCenter);
    layout->addWidget( temp_label );
    temp_label = new QLabel( m_lv->getFilesystemLabel() );
    temp_label->setToolTip( m_lv->getFilesystemLabel() );
    temp_label->setWordWrap(true);
    layout->addWidget( temp_label );

    temp_label = new QLabel( i18n("<b>Filesystem UUID</b>") );
    temp_label->setAlignment(Qt::AlignCenter);
    layout->addWidget( temp_label );
    temp_label = new QLabel( m_lv->getFilesystemUuid() );
    temp_label->setToolTip( m_lv->getFilesystemUuid() );
    temp_label->setWordWrap(true);
    layout->addWidget( temp_label );

    return frame;
}

QFrame *LVProperties::generalFrame(int segment)
{
    const long long extent_size = m_lv->getVG()->getExtentSize();
    const int segment_count = m_lv->getSegmentCount();
    long long extents, total_size, total_extents;
    int stripes, stripe_size;
    QStringList pv_list;
    QLabel *temp_label;

    QFrame *frame = new QFrame();
    QVBoxLayout *layout = new QVBoxLayout();
    QHBoxLayout *stripe_layout = new QHBoxLayout();
    frame->setLayout(layout);
    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);

    if((segment >= 0) && (segment_count > 1)){

        temp_label = new QLabel( "<b>" + m_lv->getName() + QString("[%1]").arg(segment) + "</b>" );
	temp_label->setAlignment(Qt::AlignCenter);
	layout->addWidget(temp_label);

	extents = m_lv->getSegmentExtents(segment);
	stripes = m_lv->getSegmentStripes(segment);
	stripe_size = m_lv->getSegmentStripeSize(segment);

	layout->addWidget(new QLabel( i18n("Extents: %1", extents) ) );

	if( !m_lv->isMirror() ){

	    if( stripes != 1 ){
		stripe_layout->addWidget(new QLabel( i18n("Stripes: %1", stripes) ));
                stripe_layout->addWidget(new QLabel( i18n("Stripe size: %1", stripe_size) ));
	    }
	    else{
		stripe_layout->addWidget(new QLabel( i18n("Stripes: none") ));
	    }

            layout->addLayout(stripe_layout);
	}
    }
    else if((segment >= 0) && (segment_count == 1)){

        if(m_lv->isSnapContainer())
            temp_label = new QLabel( "<b>" + m_lv->getName() + " + Snapshots </b>" );
        else
            temp_label = new QLabel( "<b>" + m_lv->getName() + "</b>" );

	temp_label->setAlignment(Qt::AlignCenter);
	layout->addWidget(temp_label);
	extents = m_lv->getSegmentExtents(segment);
        total_size = m_lv->getTotalSize();
        total_extents = total_size / extent_size;
	stripes = m_lv->getSegmentStripes(segment);
	stripe_size = m_lv->getSegmentStripeSize(segment);

        if( !m_lv->isSnapContainer() )
            layout->addWidget(new QLabel( i18n("Extents: %1", extents) ));

	if( !m_lv->isMirror() ){

	    if( stripes != 1 ){
		stripe_layout->addWidget(new QLabel( i18n("Stripes: %1", stripes) ));
                stripe_layout->addWidget(new QLabel( i18n("Stripe size: %1", stripe_size) ));
	    }
	    else{
		stripe_layout->addWidget(new QLabel( i18n("Stripes: none") ));
	    }

            layout->addLayout(stripe_layout);
	}
	else if( !m_lv->isMirrorLog() || ( m_lv->isMirrorLog() && m_lv->isMirror() ) ){
            layout->addWidget(new QLabel( i18n("Total extents: %1", total_extents) ));
	    layout->addWidget(new QLabel( i18n("Total size: %1", sizeToString(total_size)) ));
	}

	if( !( m_lv->isMirrorLeg() || m_lv->isMirrorLog() )){

            layout->addWidget(new QLabel( i18n("Filesystem: %1", m_lv->getFilesystem() ) ));

	    if(m_lv->isWritable())
	        layout->addWidget(new QLabel( i18n("Access: r/w") ));
	    else
	        layout->addWidget(new QLabel( i18n("Access: r/o") ));
                                         
            layout->addWidget(new QLabel( i18n("Allocation policy: %1", m_lv->getPolicy() ) ));
	}
    }
    else{
	temp_label = new QLabel( "<b> " + m_lv->getName() + "</b>" );
	temp_label->setAlignment(Qt::AlignCenter);
	layout->addWidget(temp_label);
	extents = m_lv->getExtents();
	layout->addWidget(new QLabel( i18n("Extents: %1", extents) ));

	if( !( m_lv->isMirrorLeg() || m_lv->isMirrorLog() )){

            layout->addWidget(new QLabel( i18n("Filesystem: %1", m_lv->getFilesystem()) ));

	    if(m_lv->isWritable())
	        layout->addWidget(new QLabel( i18n("Access: r/w") ));
	    else
	        layout->addWidget(new QLabel( i18n("Access: r/o") ));

	    layout->addWidget(new QLabel( i18n("Allocation policy: %1", m_lv->getPolicy())));
	}
    }

    if(m_lv->isSnap())
        layout->addWidget(new QLabel( i18n("Origin: %1", m_lv->getOrigin()) ));

    return frame;
}

QFrame *LVProperties::physicalVolumesFrame(int segment)
{
    QStringList pv_list;
    QFrame *frame = new QFrame();
    QVBoxLayout *layout = new QVBoxLayout();

    frame->setLayout(layout);
    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);

    QLabel *temp_label = new QLabel( i18n("<b>Physical volumes</b>") );
    temp_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(temp_label);

    if( m_lv->isMirror() || m_lv->isSnapContainer() ){
	pv_list = m_lv->getPVNamesAllFlat();
	for(int pv = 0; pv < pv_list.size(); pv++){
	    temp_label = new QLabel( pv_list[pv] );
	    temp_label->setToolTip( pv_list[pv] );
	    layout->addWidget( temp_label );
	}
    } 
    else if(segment > -1){
	pv_list = m_lv->getPVNames(segment);
	for(int pv = 0; pv < pv_list.size(); pv++){
	    temp_label = new QLabel( pv_list[pv] );
	    temp_label->setToolTip( pv_list[pv] );
	    layout->addWidget( temp_label );
	}
    }
    else{
	pv_list = m_lv->getPVNamesAll();
	for(int pv = 0; pv < pv_list.size(); pv++){
	    temp_label = new QLabel( pv_list[pv] );
	    temp_label->setToolTip( pv_list[pv] );
	    layout->addWidget( temp_label );
	}
    }

    return frame;
}
