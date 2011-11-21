/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "pvproperties.h"

#include <KSeparator>
#include <KLocale>
#include <QtGui>

#include "masterlist.h"
#include "misc.h"
#include "physvol.h"
#include "logvol.h"
#include "volgroup.h"

struct LVSegmentExtent
{
    QString lv_name;
    long long first_extent;
    long long last_extent;
};

bool isLessThan( const LVSegmentExtent * lv1 , const LVSegmentExtent * lv2 )
{
    return lv1->first_extent < lv2->first_extent;
}


PVProperties::PVProperties(PhysVol *physicalVolume, QWidget *parent) : 
    QWidget(parent),
    m_pv(physicalVolume)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    QVBoxLayout *const top_layout = new QVBoxLayout;
    top_layout->setSpacing(2);
    top_layout->setMargin(2);
    top_layout->addWidget( buildLVBox() );
    top_layout->addWidget( buildMDABox() );
    top_layout->addWidget( buildUUIDBox() );
    top_layout->addStretch();
    setLayout(top_layout);
} 

QList<LVSegmentExtent *> PVProperties::sortByExtent()
{
    const QString pv_name = m_pv->getName();
    VolGroup *const vg = m_pv->getVG();
    const QList<LogVol *>  lvs = vg->getLogicalVolumesFlat();
    QList<long long> first_extent_list;
    QStringList pv_name_list;
    QList<LVSegmentExtent *> lv_extents;
    LVSegmentExtent *temp;
    LogVol *lv;

    for(int x = 0; x < lvs.size() ; x++){
	lv = lvs[x];
        if( !lv->isSnapContainer() ){
            for(int segment = lv->getSegmentCount() - 1; segment >= 0; segment--){
                pv_name_list = lv->getPVNames(segment);
                first_extent_list = lv->getSegmentStartingExtent(segment);
                for(int y = pv_name_list.size() - 1; y >= 0; y--){
                    if( pv_name_list[y] == pv_name ){
                        temp = new LVSegmentExtent;
                        temp->lv_name = lv->getName();
                        temp->first_extent = first_extent_list[y];
                        temp->last_extent = temp->first_extent - 1 + (lv->getSegmentExtents(segment) / (lv->getSegmentStripes(segment)));
                        lv_extents.append(temp);
                    }
                }
            }
        }
    }

    qSort( lv_extents.begin() , lv_extents.end(), isLessThan );

    return lv_extents;
}

QFrame *PVProperties::buildMDABox()
{
    QLabel *temp_label;
    QHBoxLayout *const layout = new QHBoxLayout;
    QFrame *const frame = new QFrame;
    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);
    frame->setLayout(layout);

    temp_label = new QLabel( "<b>MDA</b>" );
    temp_label->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
    layout->addWidget(temp_label);
    temp_label = new QLabel( QString("Total: %1").arg( m_pv->getMDACount() ) );
    layout->addWidget(temp_label);
    temp_label = new QLabel( QString("In use: %1").arg( m_pv->getMDAUsed() ) );
    layout->addWidget(temp_label);
    temp_label = new QLabel( QString("Size: %1").arg( sizeToString( m_pv->getMDASize() ) ) );
    layout->addWidget(temp_label);

    return frame;
}

QFrame *PVProperties::buildLVBox()
{
    QGridLayout *const layout = new QGridLayout;
    QLabel *temp_label;
    long long first_extent;
    long long last_extent;

    QFrame *const frame = new QFrame;
    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);
    frame->setLayout(layout);

    temp_label = new QLabel( i18n("Volume name") );
    temp_label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(temp_label, 1, 0);
    temp_label =  new QLabel( i18n("Start") );
    temp_label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(temp_label, 1, 1);
    temp_label = new QLabel( i18n("End") );
    temp_label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(temp_label, 1, 2);
    temp_label =  new QLabel( i18n("Extents") );
    temp_label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(temp_label, 1, 3);

    const QList<LVSegmentExtent *> lv_extents = sortByExtent();

    int row = 2;

    for(int x = 0; x < lv_extents.size() ; x++){

        first_extent = lv_extents[x]->first_extent;
        last_extent = lv_extents[x]->last_extent;

        if( row % 2 == 0 ){		    
            temp_label = new QLabel( lv_extents[x]->lv_name );
            temp_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter );
            temp_label->setBackgroundRole(QPalette::Base);
            temp_label->setAutoFillBackground(true);
            layout->addWidget(temp_label,row, 0);
            
            temp_label = new QLabel( i18n("%1", first_extent) );
            temp_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter );
            temp_label->setBackgroundRole(QPalette::Base);
            temp_label->setAutoFillBackground(true);
            layout->addWidget(temp_label,row, 1);
            
            temp_label = new QLabel( i18n("%1", last_extent) );
            temp_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter );
            temp_label->setBackgroundRole(QPalette::Base);
            temp_label->setAutoFillBackground(true);
            layout->addWidget(temp_label, row, 2);
            
            temp_label = new QLabel( i18n("%1", last_extent - first_extent + 1 ) );
            temp_label->setBackgroundRole(QPalette::Base);
            temp_label->setAutoFillBackground(true);
            temp_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter );
            layout->addWidget(temp_label, row, 3);
        }
        else{
            temp_label = new QLabel( lv_extents[x]->lv_name );
            temp_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter );
            temp_label->setBackgroundRole(QPalette::AlternateBase);
            temp_label->setAutoFillBackground(true);
            layout->addWidget(temp_label,row, 0);
            
            temp_label = new QLabel( i18n("%1", first_extent) );
            temp_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter );
            temp_label->setBackgroundRole(QPalette::AlternateBase);
            temp_label->setAutoFillBackground(true);
            layout->addWidget(temp_label,row, 1);
            
            temp_label = new QLabel( i18n("%1", last_extent) );
            temp_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter );
            temp_label->setBackgroundRole(QPalette::AlternateBase);
            temp_label->setAutoFillBackground(true);
            layout->addWidget(temp_label, row, 2);
            
            temp_label = new QLabel( i18n("%1", last_extent - first_extent + 1 ) );
            temp_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter );
            temp_label->setBackgroundRole(QPalette::AlternateBase);
            temp_label->setAutoFillBackground(true);
            layout->addWidget(temp_label, row, 3);
        }
        row++;
    }

    temp_label = new QLabel();
    layout->addWidget(temp_label, row + 1, 0, 1, -1 );
    temp_label = new QLabel( i18n("Total extents: %1", m_pv->getSize() / m_pv->getVG()->getExtentSize() ) );
    layout->addWidget(temp_label, row + 2, 0, 1, -1 );

    return frame;
}

QFrame *PVProperties::buildUUIDBox()
{
    QLabel *temp_label;
    QHBoxLayout *const layout = new QHBoxLayout;
    QFrame *const frame = new QFrame;
    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);
    frame->setLayout(layout);

    temp_label = new QLabel( "<b>UUID</b>" );
    temp_label->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
    layout->addWidget(temp_label);

    temp_label = new QLabel( m_pv->getUuid() );
    temp_label->setAlignment( Qt::AlignCenter );
    temp_label->setWordWrap(true);
    layout->addWidget(temp_label);

    return frame;
}

