/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "lvactionsmenu.h"

#include "logvol.h"
#include "lvactions.h"
#include "volgroup.h"

#include <KAction>
#include <KActionCollection>
#include <KLocale>

#include <QDebug>


LVActionsMenu::LVActionsMenu(LVActions *const lvactions, QWidget *parent) : 
    KMenu(parent)
{
    addAction(lvactions->action("lvcreate"));
    addAction(lvactions->action("thinpool"));
    addAction(lvactions->action("thincreate"));
    addSeparator();
    addAction(lvactions->action("lvremove"));
    addSeparator();
    addAction(lvactions->action("lvrename"));
    addAction(lvactions->action("snapcreate"));
    addAction(lvactions->action("thinsnap"));
    addAction(lvactions->action("snapmerge"));
    addAction(lvactions->action("lvreduce"));
    addAction(lvactions->action("lvextend"));
    addAction(lvactions->action("pvmove"));
    addAction(lvactions->action("pvmove"));
    addAction(lvactions->action("lvchange"));
    addSeparator();

    KMenu *const raid_menu = new KMenu(i18n("Mirrors and RAID"), this);
    raid_menu->addAction(lvactions->action("addlegs"));
    raid_menu->addAction(lvactions->action("changelog"));
    raid_menu->addAction(lvactions->action("removemirror"));
    raid_menu->addAction(lvactions->action("removethis"));
    raid_menu->addSeparator();
    raid_menu->addAction(lvactions->action("repairmissing"));
    raid_menu->addAction(lvactions->action("resync"));
    addMenu(raid_menu);

    KMenu *const fs_menu = new KMenu(i18n("Filesystem operations"), this);
    fs_menu->addAction(lvactions->action("mount"));
    fs_menu->addAction(lvactions->action("unmount"));
    fs_menu->addSeparator();
    fs_menu->addAction(lvactions->action("maxfs"));
    fs_menu->addAction(lvactions->action("fsck"));
    fs_menu->addSeparator();
    fs_menu->addAction(lvactions->action("mkfs"));

    addMenu(fs_menu);
}
