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

#ifndef LVACTIONSMENU_H
#define LVACTIONSMENU_H

#include <KMenu>
#include <KAction>

#include <QPoint>

class LogVol;
class LVChartSeg;
class VGTree;

class LVActionsMenu : public KMenu
{
Q_OBJECT

    KAction *lv_remove_action, *lv_create_action, *pv_move_action,
	    *lv_reduce_action, *lv_extend_action, *lv_change_action, *add_mirror_action, 
            *remove_mirror_action, *remove_mirror_leg_action, *snap_create_action,
            *mount_filesystem_action, *unmount_filesystem_action, *lv_mkfs_action;
    
    KMenu *filesystem_menu;

    void setup(LogVol *lv);
    
 public:
    LVActionsMenu(LogVol *logicalVolume, VGTree *volumeGroupTree, QWidget *parent);
    LVActionsMenu(LogVol *logicalVolume, LVChartSeg *chartSeg, QWidget *parent);

};

#endif
