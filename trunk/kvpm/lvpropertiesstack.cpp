/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
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

#include "lvproperties.h"
#include "logvol.h"
#include "volgroup.h"

#include <KLocale>

#include <QDebug>
#include <QElapsedTimer>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QScrollArea>
#include <QScrollBar>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QVBoxLayout>


/* Each logical volume gets a stack of widgets. The widgets display
   information about the volume highlighted on the tree widget. One
   widget for each line (segment) on the tree. The stacks are in turn
   loaded onto a stack for the whole group. */

LVPropertiesStack::LVPropertiesStack(VolGroup *Group, QWidget *parent)
    : QFrame(parent),
      m_vg(Group)
{
    m_vscroll = new QScrollArea;
    m_stack_widget = NULL;

    QVBoxLayout *const vlayout = new QVBoxLayout();
    QHBoxLayout *const hlayout = new QHBoxLayout();
    vlayout->setMargin(0);
    vlayout->setSpacing(0);

    m_lv_label = new QLabel;
    m_lv_label->setAlignment(Qt::AlignCenter);

    vlayout->addSpacing(2);
    vlayout->addWidget(m_lv_label);
    vlayout->addSpacing(2);
    vlayout->addWidget(m_vscroll);
    vlayout->addLayout(hlayout);

    setLayout(vlayout);

    m_vscroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_vscroll->setBackgroundRole(QPalette::Base);
    m_vscroll->setAutoFillBackground(true);
    m_vscroll->verticalScrollBar()->setBackgroundRole(QPalette::Window);
    m_vscroll->verticalScrollBar()->setAutoFillBackground(true);

    m_vscroll->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_vscroll->setWidgetResizable(true);
    m_vscroll->setFrameShape(QFrame::NoFrame);
}


/* If *item points to a logical volume we set the widget stack to the widget with that volume's information.
   Else we set the stack widget index to -1, nothing  */

void LVPropertiesStack::changeLVStackIndex(QTreeWidgetItem *item, QTreeWidgetItem*)
{
    QString lv_name;
    const LogVolList members  = m_vg->getLogicalVolumesFlat();

    if (item && (members.size() == m_lv_stack_list.size())) {   // These *should* be equal
        const QString lv_uuid = QVariant(item->data(2, Qt::UserRole)).toString();

        for (int x = members.size() - 1; x >= 0; --x) {
            if (lv_uuid == (members[x])->getUuid()) {

                m_stack_widget->setCurrentIndex(x);
                const int segment = QVariant(item->data(1, Qt::UserRole)).toInt();

                if (segment == -1) {
                    m_lv_stack_list[x]->setCurrentIndex(0);
                    lv_name = item->data(0, Qt::UserRole).toString();
                } else {
                    m_lv_stack_list[x]->setCurrentIndex(segment + 1);
                    if (item->data(3, Qt::UserRole).toString() == "segment")
                        lv_name = QString("%1 (%2 %3)").arg(item->data(0, Qt::UserRole).toString()).arg(i18n("segment")).arg(segment);
                    else
                        lv_name = item->data(0, Qt::UserRole).toString();
                }
                if (item->data(4, Qt::UserRole).toBool())
                    m_lv_label->setText("<b>" + lv_name + QString(" + %1").arg(i18n("Snapshots")) + "</b>");
                else
                    m_lv_label->setText("<b>" + lv_name + "</b>");
            }
        }
    } else {
        m_stack_widget->setCurrentIndex(-1);
        m_lv_label->setText("");
    }
}

void LVPropertiesStack::loadData()
{
    const LogVolList members = m_vg->getLogicalVolumesFlat();

    if (m_stack_widget == NULL)
        m_stack_widget = new QStackedWidget;

    for (int x = m_stack_widget->count() - 1; x >= 0; --x) {
        QWidget *widget = m_stack_widget->widget(x);
        m_stack_widget->removeWidget(widget);
        widget->deleteLater();
    }
    m_lv_stack_list.clear();

    for (int x = 0; x < members.size(); ++x) {

        QStackedWidget *const segment_properties_stack = new QStackedWidget();

        if (members[x]->getSegmentCount() > 1) {

            segment_properties_stack->addWidget(new LVProperties(members[x], -1));
            for (int segment = 0; segment < members[x]->getSegmentCount(); segment++)
                segment_properties_stack->addWidget(new LVProperties(members[x], segment));
        } else
            segment_properties_stack->addWidget(new LVProperties(members[x], 0));

        m_lv_stack_list.append(segment_properties_stack);
        m_stack_widget->addWidget(segment_properties_stack);
    }

    if (members.size())
        m_stack_widget->setCurrentIndex(0);

    if (m_vscroll->widget() == 0)
        m_vscroll->setWidget(m_stack_widget);

    setMinimumWidth(minimumSizeHint().width() + 18);
}

