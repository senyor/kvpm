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


#include <KSeparator>
#include <QtGui>

#include "logvol.h"
#include "lvsizechart.h"
#include "lvsizechartseg.h"
#include "volgroup.h"

LVSizeChart::LVSizeChart(VolGroup *VolumeGroup, QWidget *parent) : 
    QFrame(parent),
    m_vg(VolumeGroup)
{

    setFrameStyle(QFrame::Sunken | QFrame::Box);
    setLineWidth(2);

    m_layout = new QHBoxLayout(this);
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    m_layout->setSizeConstraint(QLayout::SetNoConstraint);
    
    populateChart();
    setLayout(m_layout);
    setMinimumHeight(45);
    setMaximumHeight(75);
}

void LVSizeChart::populateChart()
{
    double seg_ratio;
    QWidget *widget;

    long long free_extents  = m_vg->getFreeExtents();
    long long total_extents = m_vg->getExtents();
    
    QList<LogVol *> logical_volumes = m_vg->getLogicalVolumes();
    int lv_count = m_vg->getLogVolCount();
    
    QString usage;                   // Filesystem: blank, ext2 etc. or freespace in vg
    int max_segment_width;
    
    for(int x = 0; x < lv_count; x++){
	m_lv = logical_volumes[x];

	if( !m_lv->isMirrorLeg() && !m_lv->isMirrorLog() ){
	    if( m_widgets.size() ){
		KSeparator *separator = new KSeparator(Qt::Vertical);
		separator->setFrameStyle(QFrame::Sunken | QFrame::Box);
		separator->setLineWidth(2);
		separator->setMaximumWidth(2);
		m_layout->addWidget(separator);
	    }

	    usage = m_lv->getFilesystem();
	    seg_ratio = m_lv->getExtents() / (double) total_extents;

	    if( m_lv->isMirror() )
		seg_ratio *= m_lv->getSegmentStripes(0);
	
	    m_ratios.append(seg_ratio);
	    widget = new LVChartSeg(m_vg, m_lv, usage, this);
	    m_layout->addWidget(widget);
	    m_widgets.append(widget);
	}
    }

    if( free_extents ){ // only create a free space widget if we have some

	if( m_widgets.size() ){
	    KSeparator *separator = new KSeparator(Qt::Vertical);
	    separator->setFrameStyle(QFrame::Sunken | QFrame::Box);
	    separator->setLineWidth(2);
	    separator->setMaximumWidth(2);
	    m_layout->addWidget(separator);
	}
	
	seg_ratio = free_extents / (double) total_extents;
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
	max_segment_width = (int) ( (width() * m_ratios[x]) - 2 );
	
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
	max_segment_width = (int) ( (width() * m_ratios[x]) - 2 );

	if( max_segment_width < 1 )
	    max_segment_width = 1;

	m_widgets[x]->setMaximumWidth( max_segment_width ); 
    }
}
