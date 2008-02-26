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
#include "lvpropertiesstack.h"
#include "lvproperties.h"
#include "logvol.h"
#include "volgroup.h"

/* Each logical volume gets a stack of widgets. The widgets display
   information about the volume highlighted on the tree widget. One
   widget for each line (segment) on the tree. The stacks are in turn
   loaded onto a stack for the whole group. */ 

LVPropertiesStack::LVPropertiesStack(VolGroup *Group, QWidget *parent) : QStackedWidget(parent)
{
    vg = Group;
    QList<LogVol *> members  = vg->getLogicalVolumes();

    for(int x = 0; x < members.size(); x++){
	QStackedWidget *segment_properties_stack = new QStackedWidget();
	lv_stack_list.append(segment_properties_stack);
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

    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);



}

void LVPropertiesStack::changeLVStackIndex(QTreeWidgetItem *item, int)
{
/* If *item points to a volume we set the widget stack to the
   widget with that volume's information.
   If *item points to nothing but volumes exist we go to the
   first stack widget.
   Else we set the stack widget index to -1, nothing */ 

    QString lv_name;
    int segment;
    
    QList<LogVol *> members  = vg->getLogicalVolumes();
    if(item){
	lv_name = QVariant(item->data(0, Qt::UserRole)).toString();
	for(int x = 0; x < members.size(); x++)
	    if(lv_name == (members[x])->getName()){
		setCurrentIndex(x);
		segment = QVariant(item->data(1, Qt::UserRole)).toInt();
		if(segment == -1)
		    lv_stack_list[x]->setCurrentIndex(0);
		else
		    lv_stack_list[x]->setCurrentIndex(segment + 1);
	    }
    }
    else if( members.size() )
	setCurrentIndex(0);
    else
	setCurrentIndex(-1);
}
