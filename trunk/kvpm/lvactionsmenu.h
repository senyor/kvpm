/*
 *
 * 
 * Copyright (C) 2008, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef LVACTIONSMENU_H
#define LVACTIONSMENU_H

#include <KMenu>
#include <KAction>

#include <QPoint>

class VolGroup;
class LogVol;
class LVChartSeg;
class VGTree;

class LVActionsMenu : public KMenu
{
Q_OBJECT

    KAction *lv_remove_action, *lv_create_action, *pv_move_action, *lv_rename_action,
	    *lv_reduce_action, *lv_extend_action, *lv_change_action, *add_mirror_action, 
            *remove_mirror_action, *remove_mirror_leg_action, *snap_create_action,
            *mount_filesystem_action, *unmount_filesystem_action, 
            *lv_removefs_action, *lv_mkfs_action, *lv_maxfs_action;
    
    KMenu *filesystem_menu;
    VolGroup *m_vg;
    LogVol *m_lv, *m_mirror_origin;

 public:
    LVActionsMenu(LogVol *logicalVolume, VolGroup *volumeGroup, QWidget *parent);

 private slots:
    void createLogicalVolume();
    void extendLogicalVolume();
    void changeLogicalVolume();
    void reduceLogicalVolume();
    void removeLogicalVolume();
    void renameLogicalVolume();
    void addMirror();
    void removefsLogicalVolume();
    void removeMirror();
    void removeMirrorLeg();
    void createSnapshot();
    void mkfsLogicalVolume();
    void maxfsLogicalVolume();
    void movePhysicalExtents();
    void mountFilesystem();
    void unmountFilesystem();

};

#endif
