/*
 *
 *
 * Copyright (C) 2008, 2010, 2012, 2013, 2016 Benjamin Scott   <benscott@nwlink.com>
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
#include <QElapsedTimer>
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
    m_stack_widget = new QStackedWidget();
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

    m_vscroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_vscroll->setBackgroundRole(QPalette::Base);
    m_vscroll->setAutoFillBackground(true);
    m_vscroll->verticalScrollBar()->setBackgroundRole(QPalette::Window);
    m_vscroll->verticalScrollBar()->setAutoFillBackground(true);
    m_vscroll->setFrameShape(QFrame::NoFrame);

    setLayout(vlayout);
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

        for (int x = devices.size() - 1; x >= 0; --x) {
            if (pv_uuid == devices[x]->getUuid()) {
                m_stack_widget->setCurrentIndex(x);
                m_pv_label->setText("<b>" + devices[x]->getMapperName() + "</b>");
            }
        }
    } else {
        m_stack_widget->setCurrentIndex(-1);
        m_pv_label->setText("");
    }
}

void PVPropertiesStack::loadData()
{
    const QList<PhysVol *> volumes  = m_vg->getPhysicalVolumes();
    if (!m_stack_widget)
        m_stack_widget = new QStackedWidget;

    for (int x = m_stack_widget->count() - 1; x >=0; --x) {
        QWidget *widget = m_stack_widget->widget(x);
        m_stack_widget->removeWidget(widget);
        widget->deleteLater();
    }

    for (auto pv : volumes)
        m_stack_widget->addWidget(new PVProperties(pv));

    if (m_vscroll->widget() == 0)
        m_vscroll->setWidget(m_stack_widget);

    if (!volumes.isEmpty())
        m_stack_widget->setCurrentIndex(0);

    m_vscroll->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_vscroll->setWidgetResizable(true);
}

