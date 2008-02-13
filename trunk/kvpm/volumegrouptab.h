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
#ifndef TOPWINDOWTAB_H
#define TOPWINDOWTAB_H

#include <QString>
#include <QVBoxLayout>
#include <QMainWindow>
#include <QScrollArea>
#include <QTabWidget>
#include <QStackedWidget>
#include <QTreeWidgetItem>
#include "lvpropertiesstack.h"
#include "pvtree.h"
#include "vgtree.h"
 
class MasterList;
class VolGroup;

class VolumeGroupTab : public QWidget
{
Q_OBJECT
    bool is_vg;
    VolGroup *vg;
    QString group_name;
    LVPropertiesStack *lv_properties_stack;
    PVTree *pv_tree;
    VGTree *vg_tree;
    
 public:
    VolumeGroupTab(VolGroup *VolumeGroup, QWidget *parent = 0);
    VolGroup* getVolumeGroup();
    QString getVolumeGroupName();

};

#endif
