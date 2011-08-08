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


#include <QtGui>

#include "logvol.h"
#include "lvsizechart.h"
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
    QTreeWidgetItem *item, *child_item;
    LogVol *child_volume;
    QLayoutItem *child;  // remove old children of layout
    while( (child = m_layout->takeAt(0) ) != 0 )
        delete child;

    for(int x = 0; x < m_vg_tree->topLevelItemCount(); x++){ // get volumes as sorted by vgtree
        item = m_vg_tree->topLevelItem(x);
        logical_volumes.append( m_vg->getLogVolByName( item->data(0,Qt::UserRole).toString() ) );
        if( item->childCount() ){
            for(int y = 0; y < item->childCount(); y++){
                child_item = item->child(y);
                if( (child_volume = m_vg->getLogVolByName( child_item->data(0,Qt::UserRole).toString() ) ) ){
                    if( child_volume->isSnap() )
                        logical_volumes.append( m_vg->getLogVolByName(child_item->data(0,Qt::UserRole).toString()) );
                }
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
	    !(m_lv->isMirror() && m_lv->getOrigin() != "" ) ){

	    usage = m_lv->getFilesystem();

            if( m_lv->isMirror() ) 
                seg_ratio = ( m_lv->getExtents() + m_lv->getLogCount() ) / (double) total_extents;
            else
                seg_ratio = m_lv->getExtents() / (double) total_extents;

	    if( m_lv->isUnderConversion() )
	        seg_ratio *= ( m_lv->getSegmentStripes(0) + 1);
	    else if( m_lv->isMirror() )
	        seg_ratio *= m_lv->getSegmentStripes(0); // number of mirror legs

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