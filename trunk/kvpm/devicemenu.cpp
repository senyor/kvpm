/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "devicemenu.h"

#include <KLocalizedString>

#include <QAction>
#include <QActionGroup>
#include <QIcon>

#include "deviceactions.h"



DeviceMenu::DeviceMenu(DeviceActions *const devacts, QWidget *parent) :
    QMenu(parent)
{
    QMenu *const filesystem_menu = new QMenu(i18n("Filesystem operations"), this);
    QMenu *const vgextend_menu = new QMenu(i18n("Extend volume group"), this);
    vgextend_menu->setIcon(QIcon::fromTheme(QStringLiteral("add")));

    addAction(devacts->action("tablecreate"));
    addSeparator();
    addAction(devacts->action("partremove"));
    addAction(devacts->action("partadd"));
    addAction(devacts->action("partchange"));
    addAction(devacts->action("changeflags"));
    addAction(devacts->action("max_pv"));
    addSeparator();
    addAction(devacts->action("vgcreate"));
    addAction(devacts->action("vgreduce"));
    addMenu(vgextend_menu);
    addSeparator();
    addMenu(filesystem_menu);
    filesystem_menu->addAction(devacts->action("mount"));
    filesystem_menu->addAction(devacts->action("unmount"));
    filesystem_menu->addSeparator();
    filesystem_menu->addAction(devacts->action("max_fs"));
    filesystem_menu->addAction(devacts->action("fsck"));
    filesystem_menu->addSeparator();
    filesystem_menu->addAction(devacts->action("mkfs"));

    if (!devacts->actionGroups().isEmpty()) {
        QActionGroup *vgextend_actions = devacts->actionGroups()[0];

        for (auto action : vgextend_actions->actions()) {
            vgextend_menu->addAction(action);
        }
    }
}




