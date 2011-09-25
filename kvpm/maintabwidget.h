/*
 *
 * 
 * Copyright (C) 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef MAINTABWIDGET_H
#define MAINTABWIDGET_H

#include <QIcon>
#include <QStringList>
#include <KTabWidget>

class VolumeGroupTab;
class DeviceTab;

class MainTabWidget : public QWidget
{
Q_OBJECT

    QStringList m_unmunged_text;    // Tab labels without amperands
    KTabWidget *m_tab_widget;
    QList<VolumeGroupTab *>  m_vg_tabs;

 signals:
    void currentIndexChanged();

 public:
    MainTabWidget(QWidget *parent = 0);
    QString getUnmungedText(int index);
    void appendVolumeGroupTab(VolumeGroupTab *page, const QIcon &icon, const QString &label);
    void appendDeviceTab(DeviceTab *page, const QString & label);
    void deleteTab(int index);
    QWidget *getWidget(int index);
    int getCount();
    int getCurrentIndex();
    VolumeGroupTab *getVolumeGroupTab(int index);
    void setIcon(int index, const QIcon &icon);
 
 public slots:
    void indexChanged(int index);

};

#endif
