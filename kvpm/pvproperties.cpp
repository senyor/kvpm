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

#include <KSeparator>
#include <KLocale>
#include <QtGui>

#include "masterlist.h"
#include "physvol.h"
#include "pvproperties.h"
#include "logvol.h"
#include "volgroup.h"

extern MasterList *master_list;

PVProperties::PVProperties(PhysVol *physicalVolume, QWidget *parent):QWidget(parent)
{
    VolGroup *vg = physicalVolume->getVolGroup();
    QGridLayout *layout = new QGridLayout;
    LogVol *lv;
    QLabel *temp_label;
    
    long long first_extent;
    long long last_extent;
    
    QList<LogVol *>  lvs = vg->getLogicalVolumes();
    QList<PhysVol *> pvs = vg->getPhysicalVolumes();
    QList<long long> starting_extent;
    QStringList pv_name_list;
    QString device_name = physicalVolume->getDeviceName();

    layout->setMargin(0);
    temp_label = new QLabel( "<b>" + device_name + "</b>");
    temp_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(temp_label, 0, 0, 1, -1);

    temp_label = new QLabel("Volume name");
    temp_label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(temp_label, 1, 0);
    temp_label =  new QLabel("Start");
    temp_label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(temp_label, 1, 1);
    temp_label = new QLabel("End");
    temp_label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(temp_label, 1, 2);
    temp_label =  new QLabel("Extents");
    temp_label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(temp_label, 1, 3);

    setLayout(layout);

/* here we get the names of logical volumes associated
   with the physical volume */

    int row = 2;
    
    for(int x = 0; x < lvs.size() ; x++){
	lv = lvs[x];
	for(int segment = 0; segment < lv->getSegmentCount(); segment++){
	    pv_name_list = lv->getDevicePath(segment);
	    starting_extent = lv->getSegmentStartingExtent(segment);
	    for(int y = 0; y < pv_name_list.size() ; y++){
		if( device_name == pv_name_list[y] ){
		    first_extent = starting_extent[y];
		    last_extent = first_extent - 1 + (lv->getSegmentExtents(segment) / (lv->getSegmentStripes(segment)));

                    if( row % 2 == 0 ){		    
                        temp_label = new QLabel( lv->getName() );
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
                        temp_label = new QLabel( lv->getName() );
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
            }
        }
    }

    KSeparator *separator = new KSeparator(Qt::Horizontal);
    separator->setFrameStyle(QFrame::Plain | QFrame::Box);
    separator->setLineWidth(2);
    separator->setMaximumHeight(2);
    layout->addWidget(separator, row, 0, 1, -1 );
 
    temp_label = new QLabel( "<b>Physical volume UUID</b>" );
    temp_label->setAlignment( Qt::AlignCenter );
    layout->addWidget(temp_label, row + 1, 0, 1, -1 );

    temp_label = new QLabel( physicalVolume->getUuid() );
    temp_label->setAlignment( Qt::AlignCenter );
    temp_label->setWordWrap(true);
    layout->addWidget(temp_label, row + 2, 0, 1, -1 );

    layout->setRowStretch(row + 3, 10);
} 
