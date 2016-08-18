/*
 *
 *
 * Copyright (C) 2013, 2016 Benjamin Scott   <benscott@nwlink.com>
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

class QAction;
class QTreeWidgetItem;

class LogVol;
class VolGroup;


class LVActions : public KActionCollection
{
    Q_OBJECT

    VolGroup *m_vg = nullptr;
    LogVol *m_lv = nullptr;
    int m_segment = 0;

    void setActions(LogVol *const lv, const int segment);
    void setMirrorActions(LogVol *const lv);

public:
    explicit LVActions(VolGroup *const group, QWidget *parent = nullptr);

public slots:
    void changeLv(QTreeWidgetItem *item);
    void changeLv(LogVol *lv, int segment);

private slots:
    void callDialog(QAction *action);

};

#endif
