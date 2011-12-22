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
#include <KConfigSkeleton>

#include <QtGui>

#include "logvol.h"
#include "misc.h"
#include "mountentry.h"
#include "volgroup.h"


/* if segment = -1 we have a multi segement logical volume but
   we are not focused on any one segment. Therefor stripes and
   stripe size have no meaning */

LVProperties::LVProperties(LogVol *const logicalVolume, const int segment, QWidget *parent):
    QWidget(parent),
    m_lv(logicalVolume)
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(2);
    layout->setMargin(2);

    KConfigSkeleton skeleton;

    bool show_mount, 
         show_fsuuid, 
         show_fslabel, 
         show_uuid;

    skeleton.setCurrentGroup("LogicalVolumeProperties");
    skeleton.addItemBool("mount",   show_mount,   true);
    skeleton.addItemBool("fsuuid",  show_fsuuid,  false);
    skeleton.addItemBool("fslabel", show_fslabel, false);
    skeleton.addItemBool("uuid",    show_uuid,    false);

    layout->addWidget( generalFrame(segment) );

    if( !m_lv->isMirrorLeg() && !m_lv->isMirrorLog() &&
	!m_lv->isPvmove()    && !m_lv->isVirtual() &&
        !m_lv->isSnapContainer() && ( (m_lv->getSegmentCount() == 1) || (segment == -1) ) ){

        if(show_mount)
            layout->addWidget( mountPointsFrame() );

        layout->addWidget( physicalVolumesFrame(segment) );

        if(show_fsuuid || show_fslabel)
            layout->addWidget( fsFrame(show_fsuuid, show_fslabel) );
    }
    else
        layout->addWidget( physicalVolumesFrame(segment) );

    if( show_uuid && !m_lv->isSnapContainer() && ( (m_lv->getSegmentCount() == 1) || (segment == -1) ) )
        layout->addWidget( uuidFrame() );

    layout->addStretch();
    
    setLayout(layout);
}

QFrame *LVProperties::mountPointsFrame()
{
    QLabel *label;
    QFrame *const frame = new QFrame();
    QVBoxLayout *const layout = new QVBoxLayout();
    frame->setLayout(layout);
    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);

    const QList<MountEntry *> entries = m_lv->getMountEntries();

    if(entries.size() > 1){
        label = new QLabel( i18n("<b>Mount points</b>") );
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
    }
    else{
        label = new QLabel( i18n("<b>Mount point</b>") ) ;
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
    }

    if(entries.size() == 0){
        label = new QLabel( i18n("not mounted") ) ;
        label->setAlignment(Qt::AlignLeft);
        layout->addWidget(label);
    }
    else{
        for(int x = 0; x < entries.size(); x++){
            const int pos = entries[x]->getMountPosition();
            const QString mp = entries[x]->getMountPoint();

            if(pos > 0){
                label = new QLabel( QString("%1 <%2>").arg(mp).arg(pos) );
                label->setToolTip( QString("%1 <%2>").arg(mp).arg(pos) );
                layout->addWidget(label);
            }
            else{
                label = new QLabel(mp);
                label->setToolTip(mp);
                layout->addWidget(label);
            }
        }
    }

    QListIterator<MountEntry *> entry_itr(entries);
    while( entry_itr.hasNext() )
        delete entry_itr.next();

    return frame;
}

QFrame *LVProperties::uuidFrame()
{
    QStringList uuid;
    QFrame *frame = new QFrame();
    QVBoxLayout *layout = new QVBoxLayout();
    frame->setLayout(layout);
    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);

    QLabel *label = new QLabel( i18n("<b>Logical volume UUID</b>") );
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget( label );

    uuid = splitUuid( m_lv->getUuid() );
    label = new QLabel( uuid[0] );
    label->setToolTip( m_lv->getUuid() );
    layout->addWidget( label );

    label = new QLabel( uuid[1] );
    label->setToolTip( m_lv->getUuid() );
    layout->addWidget( label );

    return frame;
}

QFrame *LVProperties::fsFrame(const bool showFsUuid, const bool showFsLabel)
{
    QStringList uuid;
    QFrame *frame = new QFrame();
    QVBoxLayout *layout = new QVBoxLayout();
    frame->setLayout(layout);
    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);
    QLabel *label;

    if(showFsLabel){
        label = new QLabel( i18n("<b>Filesystem label</b>") );
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget( label );
        label = new QLabel( m_lv->getFilesystemLabel() );
        label->setToolTip( m_lv->getFilesystemLabel() );
        label->setWordWrap(true);
        layout->addWidget( label );
    }

    if(showFsUuid){
        label = new QLabel( i18n("<b>Filesystem UUID</b>") );
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget( label );
        
        uuid = splitUuid( m_lv->getFilesystemUuid() );
        label = new QLabel( uuid[0] );
        label->setToolTip( m_lv->getFilesystemUuid() );
        layout->addWidget( label );
        label = new QLabel( uuid[1] );
        label->setToolTip( m_lv->getFilesystemUuid() );
        layout->addWidget( label );
    }

    return frame;
}

QFrame *LVProperties::generalFrame(int segment)
{
    const long long extent_size = m_lv->getVg()->getExtentSize();
    const int segment_count = m_lv->getSegmentCount();
    long long extents, total_size, total_extents;
    int stripes, stripe_size;
    QStringList pv_list;

    QFrame *frame = new QFrame();
    QVBoxLayout *layout = new QVBoxLayout();
    QHBoxLayout *stripe_layout = new QHBoxLayout();
    frame->setLayout(layout);
    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);

    if((segment >= 0) && (segment_count > 1)){

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

    QLabel *label = new QLabel( i18n("<b>Physical volumes</b>") );
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    if( m_lv->isMirror() || m_lv->isSnapContainer() ){
	pv_list = m_lv->getPvNamesAllFlat();
	for(int pv = 0; pv < pv_list.size(); pv++){
	    label = new QLabel( pv_list[pv] );
	    label->setToolTip( pv_list[pv] );
	    layout->addWidget( label );
	}
    } 
    else if(segment > -1){
	pv_list = m_lv->getPvNames(segment);
	for(int pv = 0; pv < pv_list.size(); pv++){
	    label = new QLabel( pv_list[pv] );
	    label->setToolTip( pv_list[pv] );
	    layout->addWidget( label );
	}
    }
    else{
	pv_list = m_lv->getPvNamesAll();
	for(int pv = 0; pv < pv_list.size(); pv++){
	    label = new QLabel( pv_list[pv] );
	    label->setToolTip( pv_list[pv] );
	    layout->addWidget( label );
	}
    }

    return frame;
}
