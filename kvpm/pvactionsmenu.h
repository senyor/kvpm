/*
 *
 *
 * Copyright (C) 2013, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef PVACTIONSMENU_H
#define PVACTIONSMENU_H

#include <QMenu>

class PVActions;


class PVActionsMenu : public QMenu
{
    Q_OBJECT

public:
    PVActionsMenu(PVActions *const pvactions, QWidget *parent);

};

#endif
