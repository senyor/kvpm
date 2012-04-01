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

#ifndef VOLUMEGROUPTAB_H
#define VOLUMEGROUPTAB_H

#include <QString>
#include <QVBoxLayout>
#include <QScrollArea>

class VolGroup;
class LVPropertiesStack;
class PVPropertiesStack;
class PVTree;
class VGTree;
class VGInfoLabels;
class LVSizeChart;

class VolumeGroupTab : public QWidget
{
    Q_OBJECT

    LVSizeChart  *m_lv_size_chart;
    VGInfoLabels *m_vg_info_labels;
    QVBoxLayout  *m_layout;
    VolGroup     *m_vg;
    LVPropertiesStack *m_lv_properties_stack;
    PVPropertiesStack *m_pv_properties_stack;
    PVTree *m_pv_tree;
    VGTree *m_vg_tree;
    QString m_group_name;

    void readConfig();

public:
    explicit VolumeGroupTab(VolGroup *volumeGroup, QWidget *parent = 0);
    VolGroup* getVg();
    void rescan();
};

#endif
