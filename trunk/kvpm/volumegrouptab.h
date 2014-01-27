/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef VOLUMEGROUPTAB_H
#define VOLUMEGROUPTAB_H

#include <KMainWindow>

#include <QMenu>

#include "typedefs.h"

class QString;
class QVBoxLayout;
class QScrollArea;
class QTreeWidgetItem;                         \

class KToolBar;

class VolGroup;
class LVPropertiesStack;
class PVPropertiesStack;
class PVTree;
class VGTree;
class VGInfoLabels;
class VGWarning;
class LVSizeChart;
class LVActions;
class PVActions;
class QFrame;
class LogVol;

class VolumeGroupTab : public KMainWindow
{
    Q_OBJECT

    VGWarning    *m_vg_warning = nullptr; 
    LVActions    *m_lv_actions = nullptr;
    PVActions    *m_pv_actions = nullptr;
    LVSizeChart  *m_lv_size_chart = nullptr;
    VGInfoLabels *m_vg_info_labels = nullptr;
    QVBoxLayout  *m_layout = nullptr;
    VolGroup     *m_vg = nullptr;
    LVPropertiesStack *m_lv_properties_stack = nullptr;
    PVPropertiesStack *m_pv_properties_stack = nullptr;
    PVTree *m_pv_tree = nullptr;
    VGTree *m_vg_tree = nullptr;

    void readConfig();
    KToolBar *buildLvToolBar();
    KToolBar *buildPvToolBar();

public:
    explicit VolumeGroupTab(VolGroup *const group, QWidget *parent = nullptr);
    VolGroup *getVg();
    QMenu *createPopupMenu() { return nullptr; }
    void rescan();

public slots:
    void lvContextMenu(QTreeWidgetItem *item);
    void lvContextMenu(LvPtr lv);
    void pvContextMenu(QTreeWidgetItem *item);
};

#endif
