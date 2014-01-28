/*
 *
 *
 * Copyright (C) 2014 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <QList>
#include <QPointer>
#include <QSharedPointer>

class LogVol;
class PvSpace;


typedef QSharedPointer<LogVol> LvPtr;
typedef QList<LvPtr> LvList;
typedef QList<QSharedPointer<PvSpace>> PvSpaceList;



#endif
