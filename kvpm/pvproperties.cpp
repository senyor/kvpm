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

    QList<LogVol *>  lvs = vg->getLogicalVolumes();
    QList<PhysVol *> pvs = vg->getPhysicalVolumes();

    QStringList lv_name_list;
    QStringList pv_name_list;
    QString device_name = physicalVolume->getDeviceName();
    

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);


/* here we get the names of logical volumes associated
   with the physical volume */


    for(int x = 0; x < lvs.size() ; x++){
	pv_name_list = lvs[x]->getDevicePathAll();
	for(int y = 0; y < pv_name_list.size() ; y++)
	    if( device_name == pv_name_list[y] )
		lv_name_list.append( lvs[x]->getName() );
    }
    
/* next we remove duplicate entries */

    lv_name_list.sort();
    if( lv_name_list.size() > 1 )
	for(int x = lv_name_list.size() - 2; x >= 0; x--)
	    if( lv_name_list[x] == lv_name_list[x + 1] )
		lv_name_list.removeAt(x + 1);


    layout->addWidget( new QLabel( "<b>" + device_name + "</b>" ) );
    
    for(int x = 0; x < lv_name_list.size(); x++)
	layout->addWidget( new QLabel( lv_name_list[x] ) );

    layout->addStretch();
    
}
