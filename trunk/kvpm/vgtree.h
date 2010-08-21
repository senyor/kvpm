/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010 Benjamin Scott   <benscott@nwlink.com>
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
#include <KAction>

#include <QPoint>
#include <QMenu>
#include <QList>
#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "masterlist.h"

class LogVol;
class VolGroup;


class VGTree : public QTreeWidget
{
Q_OBJECT

    VolGroup *m_vg;
    LogVol   *m_lv;

    KMenu *filesystem_menu;

    KAction *lv_remove_action,   *lv_create_action, *pv_move_action,
	    *lv_reduce_action,   *lv_extend_action, *lv_change_action,
	    *add_mirror_action,  *remove_mirror_action, 
	    *snap_create_action, *mount_filesystem_action, 
	    *unmount_filesystem_action, *lv_mkfs_action;
    
    QList<QTreeWidgetItem *> m_lv_tree_items;
    QString m_vg_name;
    QString m_lv_name;
    QString m_pv_name;

    void setupContextMenu();
    void insertSegmentItems(LogVol *logicalVolume, QTreeWidgetItem *item);
    void insertMirrorLegItems(LogVol *mirrorVolume, QTreeWidgetItem *item);
    void setHiddenColumns();
    
public:
    VGTree(VolGroup *VolumeGroup);
    void loadData();

private slots:    
    void popupContextMenu(QPoint point);
    void adjustColumnWidth(QTreeWidgetItem *);

};

#endif
