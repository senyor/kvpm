/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef PVTREE_H
#define PVTREE_H

#include <QList>
#include <QStringList>
#include <QTreeWidget>

class QMenu;
class QPoint;

class VolGroup;
class LogVol;
class PhysVol;


class PVTree : public QTreeWidget
{
    Q_OBJECT

    VolGroup *m_vg;

    bool m_use_si_units;
    bool m_show_total;
    bool m_show_percent;
    bool m_show_both;
    int m_pv_warn_percent;

    QStringList getLvNames(PhysVol *const pv);
    void setViewConfig();
    void setupContextMenu();

public:
    explicit PVTree(VolGroup *const group, QWidget *parent = 0);
    void loadData();

signals:
    void pvMenuRequested(QTreeWidgetItem *item);

private slots:
    void popupContextMenu(QPoint point);

};

#endif
