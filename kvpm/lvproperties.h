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

#ifndef LVPROPERTIES_H
#define LVPROPERTIES_H

#include <QFrame>

class LogVol;

class LVProperties : public QWidget
{
    bool m_use_si_units;
    LogVol *m_lv;
    QFrame *mountPointsFrame();
    QFrame *uuidFrame();
    QFrame *fsFrame(const bool showFsUuid, const bool showFsLabel);
    QFrame *generalFrame(int segment);
    QFrame *physicalVolumesFrame(int segment);

public:
    LVProperties(LogVol *const volume, const int segment, QWidget *parent = 0);

};

#endif
