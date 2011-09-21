/*
 *
 * 
 * Copyright (C) 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
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


class NoMungeCheck;
class PhysVol;
class StorageDevice;
class StoragePartition;

class PVCheckBox: public QGroupBox
{
Q_OBJECT

    QList<PhysVol *> m_pvs;
    QList<StorageDevice *> m_devices;
    QList<StoragePartition *> m_partitions;
    QList<NoMungeCheck *> m_pv_checks; 
    QLabel *m_space_label, *m_extents_label;
    long long m_extent_size;

 public:
    explicit PVCheckBox(QList<PhysVol *> physicalVolumes, QWidget *parent = NULL); 
    PVCheckBox(QList<StorageDevice *> devices, QList<StoragePartition *> partitions, 
               long long extentSize, QWidget *parent = NULL); 

    QStringList getAllNames();    // names of all pvs displayed in the box
    QStringList getNames();       // names of *selected* pvs
    long long getUnusedSpace();   // total unused space on selected pvs
    QList<long long> getUnusedSpaceList();  // ditto
    void setExtentSize(long long extentSize);
    void disableOrigin(PhysVol *originVolume); // disable origin to prevent move from and to same pv

 signals:
    void stateChanged();

 private slots:
    void calculateSpace();

 public slots:
    void selectAll();
    void selectNone();

};

#endif
