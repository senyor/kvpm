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


#include "lvsizechart.h"

#include <QtGui>

#include "logvol.h"
#include "lvsizechartseg.h"
#include "volgroup.h"

LVSizeChart::LVSizeChart(VolGroup *VolumeGroup, QTreeWidget *vgTree, QWidget *parent) : 
    QFrame(parent),
    m_vg(VolumeGroup),
    m_vg_tree(vgTree)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    m_layout->setSizeConstraint(QLayout::SetNoConstraint);
    
    populateChart();
    setLayout(m_layout);
    setMinimumHeight(45);
    setMaximumHeight(75);

    connect(m_vg_tree->header(), SIGNAL(sectionClicked(int)),
            this , SLOT(vgtreeClicked()));
}

void LVSizeChart::populateChart()
{

    QList<LogVol *> logical_volumes;
    QTreeWidgetItem *item;
    LogVol *lv;
    QLayoutItem *child;
    long item_count = m_vg_tree->topLevelItemCount(); 

    while( (child = m_layout->takeAt(0) ) != 0 )   // remove old children of layout
        delete child;

    for(int x = 0; x < item_count; x++){

        item = m_vg_tree->topLevelItem(x);
        lv = m_vg->getLogVolByName( item->data(0,Qt::UserRole).toString() );

        if( lv != NULL ){
            logical_volumes.append(lv);
        
            if( lv->getSnapshotCount() ){
                logical_volumes.append( lv->getSnapshots() );
            }
        }
    }

    double seg_ratio;
    QWidget *widget;

    long long free_extents  = m_vg->getFreeExtents();
    long long total_extents = m_vg->getExtents();
    
    int lv_count = logical_volumes.size();
    
    QString usage;                   // Filesystem: blank, ext2 etc. or freespace in vg
    int max_segment_width;
    
    for(int x = 0; x < lv_count; x++){
	m_lv = logical_volumes[x];

 	if( !m_lv->isMirrorLeg() && 
	    !m_lv->isMirrorLog() &&
	    !m_lv->isVirtual() &&
	    !( m_lv->isMirror() && !(m_lv->getOrigin()).isEmpty() ) ){

	    usage = m_lv->getFilesystem();

            if( m_lv->isMirror() ) 
                seg_ratio = ( m_lv->getExtents() + m_lv->getLogCount() ) / (double) total_extents;
            else
                seg_ratio = m_lv->getExtents() / (double) total_extents;

	   if( m_lv->isMirror() )
	        seg_ratio *= m_lv->getMirrorCount();

	    m_ratios.append(seg_ratio);
	    widget = new LVChartSeg(m_vg, m_lv, usage, this);
	    m_layout->addWidget(widget);
	    m_widgets.append(widget);
	}
    }

    if( free_extents && !m_vg->isExported() ){ // only create a free space widget if we have some
	seg_ratio = (free_extents / (double) total_extents) + 0.02; // allow a little "stretch" 0.02
	usage = "freespace" ;
	widget = new LVChartSeg(m_vg, 0, usage, this);
	m_widgets.append(widget);
	
	m_layout->addWidget(widget);
	m_ratios.append(seg_ratio);
    }
    else if ( m_widgets.size() == 0 ){ // if we have no chart segs then put in a blank one
                                       // because lvsizechart won't work with zero segments
	usage = "" ;
	widget = new LVChartSeg(m_vg, 0, usage, this);
	m_widgets.append(widget);
	
	m_layout->addWidget(widget);
	m_ratios.append( 1.0 );
    }
    
    max_segment_width = (int) ( width() * m_ratios[0] ); 

    if( max_segment_width < 1 )
	max_segment_width = 1;

    m_widgets[0]->setMaximumWidth(max_segment_width); 

    for(int x =  m_widgets.size() - 1; x >= 1 ; x--){
	max_segment_width = qRound( ( (double)width() * m_ratios[x]) );
	
	if( max_segment_width < 1 )
	    max_segment_width = 1;
	
	m_widgets[x]->setMaximumWidth( max_segment_width ); 
    }
}

void LVSizeChart::resizeEvent(QResizeEvent *event)
{
    int new_width = (event->size()).width();
    int max_segment_width = (int) (new_width * m_ratios[0]);  

    if( max_segment_width < 1 )
	max_segment_width = 1;

    m_widgets[0]->setMaximumWidth(max_segment_width); 
    
    for(int x =  m_widgets.size() - 1; x >= 1 ; x--){
	max_segment_width = qRound( ( (double)width() * m_ratios[x]) );

	if( max_segment_width < 1 )
	    max_segment_width = 1;

	m_widgets[x]->setMaximumWidth( max_segment_width ); 
    }
}

void LVSizeChart::vgtreeClicked()
{
    populateChart();
}
