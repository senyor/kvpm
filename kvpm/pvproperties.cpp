/*
 *
 * 
 * Copyright (C) 2008, 2009 Benjamin Scott   <benscott@nwlink.com>
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

#include "masterlist.h"
#include "physvol.h"
#include "pvproperties.h"
#include "logvol.h"
#include "volgroup.h"


extern MasterList *master_list;

PVProperties::PVProperties(PhysVol *physicalVolume, QWidget *parent):
    QTableWidget(parent)
{

    VolGroup *vg = master_list->getVolGroupByName( physicalVolume->getVolumeGroupName() ) ;
    int segment_count;
    LogVol *lv;

    QTableWidgetItem *new_item;
    
    long long first_extent;
    long long last_extent;
    long long last_used_extent = 0; // last extent in use by the pv
    
    QList<LogVol *>  lvs = vg->getLogicalVolumes();
    QList<PhysVol *> pvs = vg->getPhysicalVolumes();
    QList<long long> starting_extent;
    QStringList pv_name_list;
    QString device_name = physicalVolume->getDeviceName();

    QStringList headers;
    headers << "Logical Volume" 
	    << "Start" 
	    << "End" 
	    << "Extents" 
	    << "";

    setColumnCount(4);
    setRowCount(0);

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

                    if( last_extent > last_used_extent )
                        last_used_extent = last_extent;
		    
		    insertRow( row );
		    
		    new_item = new QTableWidgetItem( lv->getName() );
		    new_item->setFlags(Qt::ItemIsEnabled);
		    new_item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter );
		    setItem(row, 0, new_item);
		    
		    new_item = new QTableWidgetItem( QString("%1").arg(first_extent) );
		    new_item->setFlags(Qt::ItemIsEnabled);
		    new_item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter );
		    setItem(row, 1, new_item);

		    new_item = new QTableWidgetItem( QString("%1").arg(last_extent) );
		    new_item->setFlags(Qt::ItemIsEnabled);
		    new_item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter );
		    setItem(row, 2, new_item);

		    new_item = new QTableWidgetItem( QString("%1").arg(last_extent - first_extent + 1 ) );
		    new_item->setFlags(Qt::ItemIsEnabled);
		    new_item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter );
		    setItem(row, 3, new_item);

		    row++;
		}
	    }
	}
    }

    physicalVolume->setLastUsedExtent( last_used_extent );
    physicalVolume->setExtentSize( vg->getExtentSize() );

    if( !rowCount() ){
	insertRow( row );
	new_item = new QTableWidgetItem( QString("<none>") );
	new_item->setFlags(Qt::ItemIsEnabled);
	new_item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter );
	setItem(row, 0, new_item);
	row++;
    }
    
    setHorizontalHeaderLabels(headers);
    resizeColumnToContents(0);
    resizeColumnToContents(1);
    resizeColumnToContents(2);
    resizeColumnToContents(3);

    insertRow( row );
    insertRow( ++row );
    insertRow( ++row );
    //    insertRow( ++row );

    QWidget *separator_widget = new QWidget();
    QVBoxLayout *separator_layout = new QVBoxLayout();
    separator_widget->setLayout(separator_layout);
    separator_layout->addStretch();
    KSeparator *separator = new KSeparator(Qt::Horizontal);
    separator->setFrameStyle(QFrame::Plain | QFrame::Box);
    separator->setLineWidth(2);
    separator->setMaximumHeight(2);
    separator_layout->addWidget(separator);
    separator_layout->addStretch();
    setCellWidget(row - 2, 0, separator_widget);
    setSpan(row - 2, 0, 1, 4 );
    resizeRowToContents(row - 2);
    
    new_item = new QTableWidgetItem( "Physical volume uuid" );
    new_item->setFlags(Qt::ItemIsEnabled);
    new_item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter );
    setItem(row - 1, 0, new_item);
    setSpan(row - 1, 0, 1, 4 );

    new_item = new QTableWidgetItem( physicalVolume->getUuid() );
    new_item->setFlags(Qt::ItemIsEnabled);
    new_item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter );
    new_item->setToolTip(physicalVolume->getUuid());
    setItem(row - 0, 0, new_item);
    setSpan(row - 0, 0, 1, 4 );

    setAlternatingRowColors(true);
    verticalHeader()->hide();
    setWordWrap(true);
    setShowGrid(false);
} 
