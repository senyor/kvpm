/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Klvm project.
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
#include <QPoint>
#include <QMenu>
#include <QTreeWidget>
#include "physvol.h"

class VolGroup;
class LogVol;


class PVTree : public QTreeWidget
{
Q_OBJECT
    QList<QTreeWidgetItem *> pv_tree_items;
    QAction *pv_move_action, *vg_reduce_action, *pv_change_action;
    QMenu *context_menu;
    QString pv_name, vg_name;
    void setupContextMenu();
    PhysVol *pv;
    
public:
    PVTree(VolGroup *VolumeGroup, QWidget *parent = 0);

private slots:    
    void popupContextMenu(QPoint point);
    void movePhysicalExtents();
    void reduceVolumeGroup();
    void changePhysicalVolume();
    
};

#endif
