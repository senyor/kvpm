/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Klvm project.
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
    vg(VolumeGroup)
{

    setFrameStyle(QFrame::Sunken | QFrame::Box);
    setLineWidth(2);

    layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->setSizeConstraint(QLayout::SetNoConstraint);
    
    populateChart();
    setLayout(layout);
    setMinimumHeight(45);
    setMaximumHeight(75);
}

void LVSizeChart::populateChart()
{
    double seg_ratio;
    QWidget *widget;

    long long free_extents  = vg->getFreeExtents();
    long long total_extents = vg->getExtents();
    
    QList<LogVol *> logical_volumes = vg->getLogicalVolumes();
    int lv_count = vg->getLogVolCount();
    
    QString usage;                   // Filesystem: blank, ext2 etc. or freespace in vg
    int max_segment_width;
    
    for(int x = 0; x < lv_count; x++){

	lv = logical_volumes[x];

	if( !lv->isMirrorLeg() ){
	    
	    if( x != 0 ){
		KSeparator *separator = new KSeparator(Qt::Vertical);
		separator->setFrameStyle(QFrame::Sunken | QFrame::Box);
		separator->setLineWidth(2);
		separator->setMaximumWidth(2);
		layout->addWidget(separator);
	    }

	    usage = lv->getFilesystem();

	    seg_ratio = lv->getExtents() / (double) total_extents;

	    if( lv->isMirror() )
		seg_ratio *= lv->getSegmentStripes(0);
	
	    ratios.append(seg_ratio);
	    widget = new LVChartSeg(vg, lv, usage, this);
	    layout->addWidget(widget);
	    widgets.append(widget);

	}
    }

// only create a free space widget if we have some space free

    if( free_extents ){

	KSeparator *separator = new KSeparator(Qt::Vertical);
	separator->setFrameStyle(QFrame::Sunken | QFrame::Box);
	separator->setLineWidth(2);
	separator->setMaximumWidth(2);
	layout->addWidget(separator);

	seg_ratio = free_extents / (double) total_extents;
	usage = "freespace" ;
	widget = new LVChartSeg(vg, 0, usage, this);
	widgets.append(widget);
	
	layout->addWidget(widget);
	ratios.append(seg_ratio);
    }

    max_segment_width = (int) ( width() * ratios[0] ); 

    if( max_segment_width < 1 )
	max_segment_width = 1;

    widgets[0]->setMaximumWidth(max_segment_width); 

    for(int x =  widgets.size() - 1; x >= 1 ; x--){

	max_segment_width = (int) ( (width() * ratios[x]) - 2 );
	
	if( max_segment_width < 1 )
	    max_segment_width = 1;
	
	widgets[x]->setMaximumWidth( max_segment_width ); 
    }
}

void LVSizeChart::resizeEvent(QResizeEvent *event)
{
    int new_width = (event->size()).width();
    int max_segment_width = (int) (new_width * ratios[0]);  

    if( max_segment_width < 1 )
	max_segment_width = 1;

    widgets[0]->setMaximumWidth(max_segment_width); 
    
    for(int x =  widgets.size() - 1; x >= 1 ; x--){
	
	max_segment_width = (int) ( (width() * ratios[x]) - 2 );

	if( max_segment_width < 1 )
	    max_segment_width = 1;

	widgets[x]->setMaximumWidth( max_segment_width ); 
    }
}
