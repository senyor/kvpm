/*
 *
 *
 * Copyright (C) 2008, 2011, 2012, 2013, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#ifndef DEVICEMENU_H
#define DEVICEMENU_H

#include <QMenu>


class DeviceActions;


class DeviceMenu : public QMenu
{
    Q_OBJECT


public:
    explicit DeviceMenu(DeviceActions *const devacts, QWidget *parent);

};

#endif
