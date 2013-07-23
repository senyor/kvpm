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

#ifndef LVACTIONS_H
#define LVACTIONS_H


#include <KActionCollection>

class KDialog;

class QTreeWidgetItem;

class LogVol;
class VolGroup;


class LVActions : public KActionCollection
{
    Q_OBJECT

    VolGroup *m_vg = nullptr;
    LogVol *m_lv = nullptr;
    int m_segment = 0;

public:
    LVActions(VolGroup *const group, QWidget *parent = nullptr);
    void setActions(LogVol *const lv, const int segment);
    void setMirrorActions(LogVol *const lv);

public slots:
    void changeLv(QTreeWidgetItem *item);
    void changeLv(LogVol *lv, int segment);

private slots:
    void createLv();
    void extendLv();
    void createThinVolume();
    void createThinPool();
    void changeLv();
    void reduceLv();
    void removeLv();
    void renameLv();
    void createSnapshot();
    void thinSnapshot();
    void mergeSnapshot();
    void movePhysicalExtents();
    void changeLog();
    void repairMissing();
    void removeMirror();
    void removeLeg();
    void addLegs();
    void makeFs();
    void checkFs();
    void maxFs();
    void mountFs();
    void unmountFs();
};

#endif
