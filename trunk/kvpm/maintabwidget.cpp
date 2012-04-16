/*
 *
 *
 * Copyright (C) 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "maintabwidget.h"

#include <KTabWidget>

#include <QDebug>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QVBoxLayout>

#include "volumegrouptab.h"



MainTabWidget::MainTabWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *const layout = new QVBoxLayout();
    m_tab_widget = new KTabWidget();
    m_tab_widget->setMovable(false);
    m_tab_widget->setTabsClosable(false);
    layout->addWidget(m_tab_widget);
    m_unmunged_text.clear();
    setLayout(layout);

    connect(m_tab_widget, SIGNAL(currentChanged(int)),
            this, SLOT(indexChanged(int)));
}

QString MainTabWidget::getUnmungedText(const int index)
{
    return m_unmunged_text[index];
}

void MainTabWidget::appendVolumeGroupTab(VolumeGroupTab *const page, const QIcon &icon, const QString &label)
{
    m_tab_widget->insertTab(m_tab_widget->count(), (QWidget *)page, icon, label);
    m_unmunged_text.append(label);
    m_vg_tabs.append(page);
}

void MainTabWidget::appendDeviceTab(DeviceTab *const page, const QString & label)
{
    m_tab_widget->insertTab(m_tab_widget->count(), (QWidget *) page, label);
    m_unmunged_text.append(label);
}

void MainTabWidget::deleteTab(const int index)
{
    m_tab_widget->widget(index)->deleteLater();
    m_tab_widget->removeTab(index);
    m_unmunged_text.removeAt(index);
    m_vg_tabs.removeAt(index - 1);
}

QWidget *MainTabWidget::getWidget(const int index)
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

VolumeGroupTab *MainTabWidget::getVolumeGroupTab(const int index)
{
    return m_vg_tabs[index];
}

void MainTabWidget::setIcon(const int index, const QIcon &icon)
{
    m_tab_widget->setTabIcon(index, icon);
}

void MainTabWidget::indexChanged(int)
{
    emit currentIndexChanged();
}

