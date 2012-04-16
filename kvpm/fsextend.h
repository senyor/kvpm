/*
 *
 *
 * Copyright (C) 2009, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef FSEXTEND_H
#define FSEXTEND_H


#include <QStringList>


bool fs_extend(const QString dev, const QString fs, const QStringList mps, const bool isLV = false);
bool fs_can_extend(const QString fs);


#endif
