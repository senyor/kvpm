/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Klvm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */
#ifndef REMOVEMISSING_H
#define REMOVEMISSING_H

#include <KDialog>
#include <QStringList>

class VolGroup;

bool remove_missing_pv(VolGroup *VolumeGroup);

#endif
