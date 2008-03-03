/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
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
#include <QPoint>
#include <QMenu>
#include <QTreeWidget>


class VolGroup;
class LogVol;
class PhysVol;


class PVTree : public QTreeWidget
{
Q_OBJECT

    QMenu *context_menu;

    QAction *pv_move_action, 
            *vg_reduce_action, 
	    *pv_change_action;

    QString m_pv_name, 
            m_vg_name;

    void setupContextMenu();
    
public:
    PVTree(VolGroup *volumeGroup, QWidget *parent = 0);

private slots:    
    void popupContextMenu(QPoint point);
    void movePhysicalExtents();
    void reduceVolumeGroup();
    void changePhysicalVolume();
    
};

#endif
