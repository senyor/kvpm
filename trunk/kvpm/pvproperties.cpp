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


#include <QtGui>

#include "masterlist.h"
#include "physvol.h"
#include "pvproperties.h"
#include "logvol.h"
#include "volgroup.h"


extern MasterList *master_list;

PVProperties::PVProperties(PhysVol *physicalVolume, QWidget *parent):QWidget(parent)
{
    VolGroup *vg = master_list->getVolGroupByName( physicalVolume->getVolumeGroupName() ) ;
    int segment_count;
    LogVol *lv;

    QTableWidgetItem *new_item;
    
    long long first_extent;
    long long last_extent;
    
    QList<LogVol *>  lvs = vg->getLogicalVolumes();
    QList<PhysVol *> pvs = vg->getPhysicalVolumes();
    QList<long long> starting_extent;
    QStringList pv_name_list;
    QString device_name = physicalVolume->getDeviceName();

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    QLabel *pv_name_label = new QLabel( "<b>" + device_name + "</b>");
    pv_name_label->setAlignment(Qt::AlignHCenter);
    layout->addWidget(pv_name_label);
    
    QTableWidget *table_widget = new QTableWidget();
    layout->addWidget(table_widget);

    QStringList headers;
    headers << "Logical Volume" 
	    << "Start" 
	    << "End" 
	    << "Extents" 
	    << "";

    table_widget->setColumnCount(4);
    table_widget->setRowCount(0);

    table_widget->setFrameShape(QFrame::NoFrame);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    pv_name_label->setAutoFillBackground(true);
    pv_name_label->setBackgroundRole(QPalette::Base);

    QVBoxLayout *uuid_layout = new QVBoxLayout();
    layout->addLayout(uuid_layout);
    
    QLabel *uuid_label1 = new QLabel("<b>UUID</b>");
    uuid_label1->setAlignment(Qt::AlignHCenter);
    uuid_label1->setAutoFillBackground(true);
    uuid_label1->setBackgroundRole(QPalette::Base);
    uuid_layout->addWidget(uuid_label1);
    
    QLabel *uuid_label2 = new QLabel( physicalVolume->getUuid() );
    uuid_label2->setAutoFillBackground(true);
    uuid_label2->setBackgroundRole(QPalette::Base);
    uuid_label2->setWordWrap(true);
    uuid_layout->addWidget(uuid_label2);
    

/* here we get the names of logical volumes associated
   with the physical volume */

    int row = 0;
    
    for(int x = 0; x < lvs.size() ; x++){
	lv = lvs[x];
	segment_count = lv->getSegmentCount();
	for(int segment = 0; segment < segment_count; segment++){
	    pv_name_list = lv->getDevicePath(segment);
	    starting_extent = lv->getSegmentStartingExtent(segment);
	    for(int y = 0; y < pv_name_list.size() ; y++){
		if( device_name == pv_name_list[y] ){
		    first_extent = starting_extent[y];
		    last_extent = first_extent - 1 + (lv->getSegmentExtents(segment) / (lv->getSegmentStripes(segment)));
		    
		    table_widget->insertRow( row );
		    
		    new_item = new QTableWidgetItem( lv->getName() );
		    new_item->setFlags(Qt::ItemIsEnabled);
		    new_item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter );
		    table_widget->setItem(row, 0, new_item);
		    
		    new_item = new QTableWidgetItem( QString("%1").arg(first_extent) );
		    new_item->setFlags(Qt::ItemIsEnabled);
		    new_item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter );
		    table_widget->setItem(row, 1, new_item);

		    new_item = new QTableWidgetItem( QString("%1").arg(last_extent) );
		    new_item->setFlags(Qt::ItemIsEnabled);
		    new_item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter );
		    table_widget->setItem(row, 2, new_item);

		    new_item = new QTableWidgetItem( QString("%1").arg(last_extent - first_extent + 1 ) );
		    new_item->setFlags(Qt::ItemIsEnabled);
		    new_item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter );
		    table_widget->setItem(row, 3, new_item);

		    row++;
		}
	    }
	}
    }

    table_widget->setHorizontalHeaderLabels(headers);
    table_widget->resizeColumnToContents(0);
    table_widget->resizeColumnToContents(1);
    table_widget->resizeColumnToContents(2);
    table_widget->resizeColumnToContents(3);
    table_widget->setShowGrid(false);
    table_widget->setAlternatingRowColors(true);
    table_widget->verticalHeader()->hide();
    table_widget->resizeRowsToContents();
//    table_widget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    table_widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    layout->setSizeConstraint(  QLayout::SetNoConstraint     );


// the uuid (sub)layout also gets its margin set to 0 when
// the main layout margin is set to 0. So we change uuid_layout
// back to the default here. Ditto for the spacing.

    int margin  = layout->margin();
    int spacing = layout->spacing();
    
    layout->setSpacing(0);
    layout->setMargin(0);

    uuid_layout->setSpacing(spacing);
    uuid_layout->setMargin(margin);

} 
