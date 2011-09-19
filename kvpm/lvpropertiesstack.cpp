/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include "lvpropertiesstack.h"

#include <QtGui>

#include "lvproperties.h"
#include "logvol.h"
#include "volgroup.h"

/* Each logical volume gets a stack of widgets. The widgets display
   information about the volume highlighted on the tree widget. One
   widget for each line (segment) on the tree. The stacks are in turn
   loaded onto a stack for the whole group. */ 

LVPropertiesStack::LVPropertiesStack(VolGroup *Group, QWidget *parent) : QStackedWidget(parent)
{
    m_vg = Group;
    QList<LogVol *> members  = m_vg->getLogicalVolumesFlat();

    for(int x = 0; x < members.size(); x++){
	QStackedWidget *segment_properties_stack = new QStackedWidget();
	m_lv_stack_list.append(segment_properties_stack);
	addWidget(segment_properties_stack);
  
	if(members[x]->getSegmentCount() > 1){
	    segment_properties_stack->addWidget(new LVProperties(members[x], -1));
	    for(int segment = 0; segment < members[x]->getSegmentCount(); segment++)
		segment_properties_stack->addWidget(new LVProperties(members[x], segment));
	}
	else{
	    segment_properties_stack->addWidget(new LVProperties(members[x], 0));
	    addWidget(segment_properties_stack);  
	}

    }
    if( members.size() )
	setCurrentIndex(0);
}

void LVPropertiesStack::changeLVStackIndex(QTreeWidgetItem *item, QTreeWidgetItem*)
{

/* If *item points to a logical volume we set the widget stack to the
   widget with that volume's information.
   If *item points to nothing but volumes exist we go to the
   first stack widget.
   Else we set the stack widget index to -1, nothing  */ 

    QString lv_uuid;
    int segment;
    QList<LogVol *> members  = m_vg->getLogicalVolumesFlat();

    if(item && ( members.size() == m_lv_stack_list.size() ) ){  // These *should* be equal
	lv_uuid = QVariant(item->data(2, Qt::UserRole)).toString();

	for(int x = members.size() - 1; x >= 0; x--){
            if(lv_uuid == ( members[x])->getUuid() ){

		setCurrentIndex(x);
		segment = QVariant(item->data(1, Qt::UserRole)).toInt();

		if(segment == -1)
		    m_lv_stack_list[x]->setCurrentIndex(0);
		else
		    m_lv_stack_list[x]->setCurrentIndex(segment + 1);
	    }
        }
    }
    else if( members.size() )
	setCurrentIndex(0);
    else
	setCurrentIndex(-1);
}
