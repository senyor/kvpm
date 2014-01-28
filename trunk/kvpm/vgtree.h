/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#ifndef VGTREE_H
#define VGTREE_H

#include <KMenu>

#include <QStringList>
#include <QTreeWidget>

class QPoint;
class QTreeWidgetItem;


class LogVol;
class VolGroup;


class VGTree : public QTreeWidget
{
    Q_OBJECT

    VolGroup *m_vg;
    bool m_init;        // is this the initial building of the tree or a reload?
    bool m_show_percent, m_show_total, m_use_si_units, m_show_both;
    int m_fs_warn_percent;

    void setupContextMenu();
    QTreeWidgetItem *loadItem(LogVol *lv, QTreeWidgetItem *item);
    void insertChildItems(LogVol *parentVolume, QTreeWidgetItem *parentItem);
    void insertSegmentItems(LogVol *logicalVolume, QTreeWidgetItem *item);
    void setViewConfig();

public:
    VGTree(VolGroup *const group);
    void loadData();

private slots:
    void popupContextMenu(QPoint point);
    void adjustColumnWidth(QTreeWidgetItem *);

signals:
    void lvMenuRequested(QTreeWidgetItem *item);

};

#endif
