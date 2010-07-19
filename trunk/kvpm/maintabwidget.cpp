/*
 *
 * 
 * Copyright (C) 2010 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <KTabWidget>
#include <QtGui>

#include "maintabwidget.h"
#include "volumegrouptab.h"

MainTabWidget::MainTabWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout();
    m_tab_widget = new KTabWidget();
    m_tab_widget->setMovable(false);
    m_tab_widget->setTabsClosable(false); 
    layout->addWidget(m_tab_widget);
    m_unmunged_text.clear();
    setLayout(layout);

    connect(m_tab_widget, SIGNAL(currentChanged(int)), 
	    this, SLOT(indexChanged(int)));
}

QString MainTabWidget::getUnmungedText(int index)
{
    return m_unmunged_text[index];
}

void MainTabWidget::appendVolumeGroupTab(VolumeGroupTab *page, const QString & label )
{
    m_tab_widget->insertTab( m_tab_widget->count(), (QWidget *)page, label );
    m_unmunged_text.append(label);
    m_vg_tabs.append(page);
    return;
}

void MainTabWidget::appendDeviceTab(DeviceTab *page, const QString & label )
{
    m_tab_widget->insertTab( m_tab_widget->count(), (QWidget *) page, label );
    m_unmunged_text.append(label);
    return;
}

void MainTabWidget::deleteTab(int index)
{
    m_tab_widget->widget(index)->deleteLater();
    m_tab_widget->removeTab(index);
    m_unmunged_text.removeAt(index);
    m_vg_tabs.removeAt(index - 1);
    return;
}

QWidget *MainTabWidget::getWidget(int index)
{
    return m_tab_widget->widget(index);
}

int MainTabWidget::getCount()
{
    return m_tab_widget->count();
}

int MainTabWidget::getCurrentIndex()
{
    return m_tab_widget->currentIndex();
}

VolumeGroupTab *MainTabWidget::getVolumeGroupTab(int index)
{
    return m_vg_tabs[index];
}

void MainTabWidget::indexChanged(int)
{
    emit currentIndexChanged();
    return;
}

