/*
 *
 * 
 * Copyright (C) 2008, 2010 Benjamin Scott   <benscott@nwlink.com>
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

#include <QString>
#include <QVBoxLayout>

class VolGroup;
class LVPropertiesStack;
class PVPropertiesStack;
class PVTree;
class VGTree;


class VolumeGroupTab : public QWidget
{
Q_OBJECT

    QVBoxLayout *m_layout;
    QWidget *m_visable_widget;
    VolGroup *m_vg;
    QString m_group_name;
    LVPropertiesStack *m_lv_properties_stack;
    PVPropertiesStack *m_pv_properties_stack;
    PVTree *m_pv_tree;
    VGTree *m_vg_tree;

 public:
    VolumeGroupTab(VolGroup *volumeGroup, QWidget *parent = 0);
    VolGroup* getVolumeGroup();
    QString getVolumeGroupName();
    void rescan();
};

#endif
