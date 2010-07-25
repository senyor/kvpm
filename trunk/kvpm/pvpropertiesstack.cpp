/*
 *
 * 
 * Copyright (C) 2008, 2010 Benjamin Scott   <benscott@nwlink.com>
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
    : QWidget(parent), m_vg(volumeGroup)
{
    QList<PhysVol *> devices  = m_vg->getPhysicalVolumes();
    m_stack_widget = new QStackedWidget;
    QVBoxLayout *vlayout = new QVBoxLayout();
    QHBoxLayout *hlayout = new QHBoxLayout();
    PVProperties  *pv_props;
    vlayout->addWidget(m_stack_widget);
    vlayout->setSizeConstraint(QLayout::SetMinimumSize);
    hlayout->setSizeConstraint(QLayout::SetMinimumSize);
    vlayout->addLayout(hlayout);
    setLayout(vlayout);

    for(int x = 0; x < devices.size(); x++){
        pv_props = new PVProperties(devices[x]);
        m_stack_widget->addWidget( pv_props );
    }

    
    if( devices.size() )
	m_stack_widget->setCurrentIndex(0);
}

/* If *item points to a volume we set the widget stack to the
   widget with that volume's information.
   If *item points to nothing but volumes exist we go to the
   first stack widget.
   Else we set the stack widget index to -1, nothing */ 


void PVPropertiesStack::changePVStackIndex(QTreeWidgetItem *item, QTreeWidgetItem*)
{

    QString pv_name;
    
    QList<PhysVol *> devices  = m_vg->getPhysicalVolumes();

    if(item){
	pv_name = QVariant( item->data(0, Qt::DisplayRole ) ).toString();
	for(int x = 0; x < devices.size(); x++){
	    
	    if(pv_name == (devices[x])->getDeviceName()){
		m_stack_widget->setCurrentIndex(x);
	    }
	}
    }
    else if( devices.size() )
	m_stack_widget->setCurrentIndex(0);
    else
	m_stack_widget->setCurrentIndex(-1);
}
