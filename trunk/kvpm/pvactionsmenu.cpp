/*
 *
 *
 * Copyright (C) 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "pvactionsmenu.h"

#include "physvol.h"
#include "pvactions.h"
#include "volgroup.h"

#include <KAction>
#include <KActionCollection>
#include <KLocale>

#include <QDebug>


PVActionsMenu::PVActionsMenu(PVActions *const pvactions, QWidget *parent) : 
    KMenu(parent)
{
    addAction(pvactions->action("pvmove"));
    addAction(pvactions->action("pvremove"));
    addAction(pvactions->action("pvchange"));
}
