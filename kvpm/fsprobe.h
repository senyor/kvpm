/*
 *
 *
 * Copyright (C) 2008, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef FSPROBE_H
#define FSPROBE_H

class QString;

QString fsprobe_getfstype2(const QString devicePath);
QString fsprobe_getfsuuid(const QString devicePath);
QString fsprobe_getfslabel(const QString devicePath);

#endif
