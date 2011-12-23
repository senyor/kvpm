/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef PVPROPERTIES_H
#define PVPROPERTIES_H


#include <QFrame>

class LVSegmentExtent;
class PhysVol;

class PVProperties : public QWidget
{
    PhysVol *m_pv;

    QList<LVSegmentExtent *> sortByExtent();
    QFrame *buildMdaBox();
    QFrame *buildLvBox();
    QFrame *buildUuidBox();

 public:
    explicit PVProperties(PhysVol *const volume, QWidget *parent = 0);

};

#endif
