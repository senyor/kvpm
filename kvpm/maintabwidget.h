/*
 *
 *
 * Copyright (C) 2010, 2011, 2012, 2016 Benjamin Scott   <benscott@nwlink.com>
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

#include <QWidget>
#include <QStringList>

class QIcon;
class QTabWidget;

class VolumeGroupTab;
class DeviceTab;

class MainTabWidget : public QWidget
{
    Q_OBJECT

    QStringList m_unmunged_text;    // Tab labels without amperands
    QTabWidget *m_tab_widget;
    QList<VolumeGroupTab *>  m_vg_tabs;

signals:
    void currentIndexChanged(int index);

public:
    MainTabWidget(QWidget *parent = 0);
    QString getUnmungedText(const int index);
    void appendVolumeGroupTab(VolumeGroupTab *const page, const QIcon &icon, const QString &label);
    void appendDeviceTab(DeviceTab *const page, const QString & label);
    void deleteTab(const int index);
    QWidget *getWidget(const int index);
    int getCount();
    int getCurrentIndex();
    VolumeGroupTab *getVolumeGroupTab(const int index);
    void setIcon(const int index, const QIcon &icon);

};

#endif
