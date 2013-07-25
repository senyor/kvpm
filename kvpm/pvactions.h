/*
 *
 *
 * Copyright (C) 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef PVACTIONS_H
#define PVACTIONS_H


#include <KActionCollection>

class KDialog;

class QTreeWidgetItem;

class PhysVol;
class VolGroup;


class PVActions : public KActionCollection
{
    Q_OBJECT

    VolGroup *m_vg = nullptr;
    PhysVol *m_pv = nullptr;

    void setActions(PhysVol *const pv, bool const isMoving);

public:
    PVActions(VolGroup *const group, QWidget *parent = nullptr);

public slots:
    void changePv(QTreeWidgetItem *item);

private slots:
    void movePhysicalExtents();
    void reduceVolumeGroup();
    void changePhysicalVolume();

};

#endif
