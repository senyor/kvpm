/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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

#include <KConfigSkeleton>
#include <KGlobal>
#include <KLocale>
#include <KSeparator>

#include <QtGui>

#include "masterlist.h"
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


PVProperties::PVProperties(PhysVol *const volume, QWidget *parent) : 
    QWidget(parent),
    m_pv(volume)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    KConfigSkeleton skeleton;
    bool mda, uuid;

    skeleton.setCurrentGroup("PhysicalVolumeProperties");
    skeleton.addItemBool("pp_mda",  mda,  true);
    skeleton.addItemBool("pp_uuid", uuid, false);

    QVBoxLayout *const top_layout = new QVBoxLayout;
    top_layout->setSpacing(2);
    top_layout->setMargin(2);

    top_layout->addWidget( buildLvBox() );

    if(mda)
        top_layout->addWidget( buildMdaBox() );
    if(uuid)
        top_layout->addWidget( buildUuidBox() );

    top_layout->addStretch();
    setLayout(top_layout);
} 

QList<LVSegmentExtent *> PVProperties::sortByExtent()
{
    const QString pv_name = m_pv->getName();
    QList<long long> first_extent_list;
    QStringList pv_name_list;
    QList<LVSegmentExtent *> lv_extents;
    LVSegmentExtent *temp;
    LogVol *lv;

    QListIterator<LogVol *> lv_itr( m_pv->getVg()->getLogicalVolumesFlat() );

    while( lv_itr.hasNext() ){
        lv = lv_itr.next();
        if( !lv->isSnapContainer() ){
            for(int segment = lv->getSegmentCount() - 1; segment >= 0; segment--){
                pv_name_list = lv->getPvNames(segment);
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

QFrame *PVProperties::buildMdaBox()
{
    bool use_si_units;
    KConfigSkeleton skeleton;
    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", use_si_units, false);

    KLocale *const locale = KGlobal::locale();
    if(use_si_units)
        locale->setBinaryUnitDialect(KLocale::MetricBinaryDialect); 
    else
        locale->setBinaryUnitDialect(KLocale::IECBinaryDialect);

    QLabel *label;
    QHBoxLayout *const layout = new QHBoxLayout;
    QFrame *const frame = new QFrame;
    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);
    frame->setLayout(layout);

    label = new QLabel( "<b>MDA</b>" );
    label->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
    layout->addWidget(label);
    label = new QLabel( QString("Total: %1").arg( m_pv->getMdaCount() ) );
    layout->addWidget(label);
    label = new QLabel( QString("In use: %1").arg( m_pv->getMdaUsed() ) );
    layout->addWidget(label);
    label = new QLabel( QString("Size: %1").arg( locale->formatByteSize( m_pv->getMdaSize() ) ) );
    layout->addWidget(label);

    return frame;
}

QFrame *PVProperties::buildLvBox()
{
    QGridLayout *const layout = new QGridLayout;
    QLabel *label;
    long long first_extent;
    long long last_extent;

    QFrame *const frame = new QFrame;
    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);
    frame->setLayout(layout);

    label = new QLabel( i18n("Volume name") );
    label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(label, 1, 0);
    label =  new QLabel( i18n("Start") );
    label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(label, 1, 1);
    label = new QLabel( i18n("End") );
    label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(label, 1, 2);
    label =  new QLabel( i18n("Extents") );
    label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(label, 1, 3);

    const QList<LVSegmentExtent *> lv_extents = sortByExtent();

    int row = 2;

    for(int x = 0; x < lv_extents.size() ; x++){

        first_extent = lv_extents[x]->first_extent;
        last_extent = lv_extents[x]->last_extent;

        if( row % 2 == 0 ){		    
            label = new QLabel( lv_extents[x]->lv_name );
            label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter );
            label->setBackgroundRole(QPalette::Base);
            label->setAutoFillBackground(true);
            layout->addWidget(label,row, 0);
            
            label = new QLabel( i18n("%1", first_extent) );
            label->setAlignment(Qt::AlignRight | Qt::AlignVCenter );
            label->setBackgroundRole(QPalette::Base);
            label->setAutoFillBackground(true);
            layout->addWidget(label,row, 1);
            
            label = new QLabel( i18n("%1", last_extent) );
            label->setAlignment(Qt::AlignRight | Qt::AlignVCenter );
            label->setBackgroundRole(QPalette::Base);
            label->setAutoFillBackground(true);
            layout->addWidget(label, row, 2);
            
            label = new QLabel( i18n("%1", last_extent - first_extent + 1 ) );
            label->setBackgroundRole(QPalette::Base);
            label->setAutoFillBackground(true);
            label->setAlignment(Qt::AlignRight | Qt::AlignVCenter );
            layout->addWidget(label, row, 3);
        }
        else{
            label = new QLabel( lv_extents[x]->lv_name );
            label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter );
            label->setBackgroundRole(QPalette::AlternateBase);
            label->setAutoFillBackground(true);
            layout->addWidget(label,row, 0);
            
            label = new QLabel( i18n("%1", first_extent) );
            label->setAlignment(Qt::AlignRight | Qt::AlignVCenter );
            label->setBackgroundRole(QPalette::AlternateBase);
            label->setAutoFillBackground(true);
            layout->addWidget(label,row, 1);
            
            label = new QLabel( i18n("%1", last_extent) );
            label->setAlignment(Qt::AlignRight | Qt::AlignVCenter );
            label->setBackgroundRole(QPalette::AlternateBase);
            label->setAutoFillBackground(true);
            layout->addWidget(label, row, 2);
            
            label = new QLabel( i18n("%1", last_extent - first_extent + 1 ) );
            label->setAlignment(Qt::AlignRight | Qt::AlignVCenter );
            label->setBackgroundRole(QPalette::AlternateBase);
            label->setAutoFillBackground(true);
            layout->addWidget(label, row, 3);
        }
        row++;
    }

    label = new QLabel();
    layout->addWidget(label, row + 1, 0, 1, -1 );
    label = new QLabel( i18n("Total extents: %1", m_pv->getSize() / m_pv->getVg()->getExtentSize() ) );
    layout->addWidget(label, row + 2, 0, 1, -1 );

    for(int x = 0; x < lv_extents.size(); x++)
        delete lv_extents[x];

    return frame;
}

QFrame *PVProperties::buildUuidBox()
{
    QLabel *label;
    QHBoxLayout *const layout = new QHBoxLayout;
    QFrame *const frame = new QFrame;
    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);
    frame->setLayout(layout);

    label = new QLabel( "<b>UUID</b>" );
    label->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
    layout->addWidget(label);

    label = new QLabel( m_pv->getUuid() );
    label->setAlignment( Qt::AlignCenter );
    label->setWordWrap(true);
    layout->addWidget(label);

    return frame;
}

