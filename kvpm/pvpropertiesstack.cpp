/*
 *
 *
 * Copyright (C) 2008, 2010, 2012 Benjamin Scott   <benscott@nwlink.com>
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

#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QStackedWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include "logvol.h"
#include "physvol.h"
#include "pvproperties.h"
#include "volgroup.h"


PVPropertiesStack::PVPropertiesStack(VolGroup *volumeGroup, QWidget *parent)
    : QFrame(parent),
      m_vg(volumeGroup)
{
    m_vscroll = new QScrollArea;
    m_stack_widget = NULL;

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

    setLayout(vlayout);
    m_vscroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_vscroll->setBackgroundRole(QPalette::Base);
    m_vscroll->setAutoFillBackground(true);
    m_vscroll->verticalScrollBar()->setBackgroundRole(QPalette::Window);
    m_vscroll->verticalScrollBar()->setAutoFillBackground(true);
}


/* If *item points to a volume we set the widget stack to the widget with that volume's information.
   Else we set the stack widget index to -1, nothing */

void PVPropertiesStack::changePVStackIndex(QTreeWidgetItem *item, QTreeWidgetItem*)
{
    const QList<PhysVol *> devices  = m_vg->getPhysicalVolumes();

    if (!m_stack_widget)
        return;

    if (item) {
        const QString pv_uuid = QVariant(item->data(0, Qt::UserRole)).toString();

        for (int x = devices.size() - 1; x >= 0; x--) {
            if (pv_uuid == devices[x]->getUuid()) {
                m_stack_widget->setCurrentIndex(x);
                m_pv_label->setText("<b>" + devices[x]->getName() + "</b>");
            }
        }
    } else {
        m_stack_widget->setCurrentIndex(-1);
        m_pv_label->setText("");
    }
}

void PVPropertiesStack::loadData()
{
    const QList<PhysVol *> devices  = m_vg->getPhysicalVolumes();

    m_stack_widget = new QStackedWidget;

    for (int x = 0; x < devices.size(); x++)
        m_stack_widget->addWidget(new PVProperties(devices[x]));

    if (devices.size())
        m_stack_widget->setCurrentIndex(0);

    m_vscroll->setWidget(m_stack_widget);
    m_vscroll->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_vscroll->setWidgetResizable(true);
    m_vscroll->setFrameShape(QFrame::NoFrame);
}
