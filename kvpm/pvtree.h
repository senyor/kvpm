/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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

    QMenu *m_context_menu;

    bool m_use_si_units;

    QAction *pv_move_action,
            *vg_reduce_action,
            *pv_change_action;

    QString m_pv_name;

    void setViewConfig();
    void setupContextMenu();

public:
    explicit PVTree(VolGroup *const group, QWidget *parent = 0);
    void loadData();

private slots:
    void popupContextMenu(QPoint point);
    void movePhysicalExtents();
    void reduceVolumeGroup();
    void changePhysicalVolume();

};

#endif
