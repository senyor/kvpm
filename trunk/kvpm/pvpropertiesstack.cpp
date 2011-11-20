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


#include "pvpropertiesstack.h"

#include <QtGui>

#include "logvol.h"
#include "physvol.h"
#include "pvproperties.h"
#include "volgroup.h"


PVPropertiesStack::PVPropertiesStack(VolGroup *volumeGroup, QWidget *parent) 
    : QScrollArea(parent), 
      m_vg(volumeGroup)
{
    QWidget *const base_widget = new QWidget();

    m_vscroll = new QScrollArea;
    m_stack_widget = NULL;

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setWidgetResizable(true);

    QVBoxLayout *const vlayout = new QVBoxLayout();
    QHBoxLayout *const hlayout = new QHBoxLayout();
    vlayout->setMargin(0);
    vlayout->setSpacing(0);

    m_pv_label = new QLabel();
    m_pv_label->setAlignment(Qt::AlignCenter);

    vlayout->addSpacing(2);
    vlayout->addWidget(m_pv_label);
    vlayout->addSpacing(2);
    vlayout->addWidget(m_vscroll);
    vlayout->addLayout(hlayout);

    base_widget->setLayout(vlayout);
    setWidget(base_widget);

    m_vscroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_vscroll->setBackgroundRole(QPalette::Base);
    m_vscroll->setAutoFillBackground(true);
}


/* If *item points to a volume we set the widget stack to the
   widget with that volume's information.
   If *item points to nothing but volumes exist we go to the
   first stack widget.
   Else we set the stack widget index to -1, nothing */ 

void PVPropertiesStack::changePVStackIndex(QTreeWidgetItem *item, QTreeWidgetItem*)
{
    QString pv_uuid;
    const QList<PhysVol *> devices  = m_vg->getPhysicalVolumes();

    if( !m_stack_widget )
        return;

    if(item){
	pv_uuid = QVariant( item->data(0, Qt::UserRole ) ).toString();

	for(int x = devices.size() - 1; x >= 0; x--){
	    if( pv_uuid == devices[x]->getUuid() ){
		m_stack_widget->setCurrentIndex(x);
                m_pv_label->setText( "<b>" + devices[x]->getName() + "</b>" );
            }
	}
    }
    else if( devices.size() )
	m_stack_widget->setCurrentIndex(0);
    else
	m_stack_widget->setCurrentIndex(-1);
}

void PVPropertiesStack::loadData()
{
    QWidget *const old_stack = m_vscroll->takeWidget();
    const QList<PhysVol *> devices  = m_vg->getPhysicalVolumes();

    if( old_stack != NULL )
        old_stack->deleteLater();

    m_stack_widget = new QStackedWidget;

    for(int x = 0; x < devices.size(); x++)
        m_stack_widget->addWidget( new PVProperties(devices[x]) ); 

    if( devices.size() )
	m_stack_widget->setCurrentIndex(0);

    m_vscroll->setWidget(m_stack_widget);
    m_vscroll->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    m_vscroll->setWidgetResizable(true);
    m_vscroll->setFrameShape(QFrame::NoFrame);
}
