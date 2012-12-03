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

#ifndef LVACTIONSMENU_H
#define LVACTIONSMENU_H

#include <KMenu>

class KAction;

class QPoint;

class VolGroup;
class LogVol;
class LVChartSeg;
class VGTree;

class LVActionsMenu : public KMenu
{
    Q_OBJECT

    VolGroup *m_vg;
    LogVol *m_lv;
    int m_segment;

    KMenu *buildMirrorMenu(); 

public:
    LVActionsMenu(LogVol *logicalVolume, int segment, VolGroup *volumeGroup, QWidget *parent);

private slots:
    void createLv();
    void extendLv();
    void createThinVolume();
    void createThinPool();
    void changeLv();
    void reduceLv();
    void removeLv();
    void renameLv();
    void addLegs();
    void changeLog();
    void repairMissing();
    void removeMirror();
    void removeLeg();
    void createSnapshot();
    void thinSnapshot();
    void makeFs();
    void checkFs();
    void maxFs();
    void mergeSnapshot();
    void movePhysicalExtents();
    void mountFs();
    void unmountFs();

};

#endif
