/*
 *
 * 
 * Copyright (C) 2010 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef PVCHECKBOX_H
#define PVCHECKBOX_H

#include <QStringList>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QVariant>

#include "misc.h"

class PhysVol;

class PVCheckBox: public QGroupBox
{
Q_OBJECT

    QList<PhysVol *> m_pvs;
    QList<NoMungeCheck *> m_pv_checks; 
    QLabel *m_space_label, *m_extents_label;
    long long m_extent_size;

 public:
    PVCheckBox(QList<PhysVol *> physicalVolumes, long long extentSize, QWidget *parent = NULL); 
    QStringList getNames();
    long long getUnusedSpace();
    QList<long long> getUnusedSpaceList();

 signals:
    void stateChanged();

 private slots:
    void selectAll();
    void selectNone();
    void calculateSpace();

};

#endif
