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

#ifndef LVACTIONSMENU_H
#define LVACTIONSMENU_H

#include <QMenu>

class LVActions;


class LVActionsMenu : public QMenu
{
    Q_OBJECT

public:
    LVActionsMenu(LVActions *const lvactions, QWidget *parent);

};

#endif
