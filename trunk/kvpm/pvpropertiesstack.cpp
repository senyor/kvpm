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

#include "logvol.h"
#include "physvol.h"
#include "pvpropertiesstack.h"
#include "pvproperties.h"
#include "volgroup.h"


PVPropertiesStack::PVPropertiesStack(VolGroup *volumeGroup, QWidget *parent) 
  : QStackedWidget(parent)
{
    m_vg = volumeGroup;
    PVProperties *pv_properties;
    
    QList<PhysVol *> devices  = m_vg->getPhysicalVolumes();

    for(int x = 0; x < devices.size(); x++){
	pv_properties = new PVProperties(devices[x]);
	
	addWidget(pv_properties);  
    }

    if( devices.size() )
	setCurrentIndex(0);
/*
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
*/


}

/* If *item points to a volume we set the widget stack to the
   widget with that volume's information.
   If *item points to nothing but volumes exist we go to the
   first stack widget.
   Else we set the stack widget index to -1, nothing */ 


void PVPropertiesStack::changePVStackIndex(QTreeWidgetItem *item, int)
{

    QString pv_name;
    
    QList<PhysVol *> devices  = m_vg->getPhysicalVolumes();

    if(item){
	pv_name = QVariant( item->data(0, Qt::DisplayRole ) ).toString();
	for(int x = 0; x < devices.size(); x++){
	    
	    if(pv_name == (devices[x])->getDeviceName()){
		setCurrentIndex(x);
	    }
	}
    }
    else if( devices.size() )
	setCurrentIndex(0);
    else
	setCurrentIndex(-1);
}
