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

#ifndef PVGROUPBOX_H
#define PVGROUPBOX_H

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QStringList>
#include <QVariant>


class NoMungeCheck;
class PhysVol;
class StorageDevice;
class StoragePartition;


class PvGroupBox: public QGroupBox
{
    Q_OBJECT

    bool m_use_si_units;
    QList<PhysVol *> m_pvs;
    QList<StorageDevice *> m_devices;
    QList<StoragePartition *> m_partitions;
    QList<NoMungeCheck *> m_pv_checks;
    QLabel *m_space_label, *m_extents_label;
    long long m_extent_size;
    QHBoxLayout *getButtons();
    void addLabelsAndButtons(QGridLayout *const layout, const int pvCount);

public:
    explicit PvGroupBox(QList<PhysVol *> volumes, QWidget *parent = NULL);
    PvGroupBox(QList<StorageDevice *> devices, QList<StoragePartition *> partitions,
               long long extentSize, QWidget *parent = NULL);

    QStringList getAllNames();    // names of all pvs displayed in the box
    QStringList getNames();       // names of *selected* pvs
    long long getRemainingSpace();   // total unused space on selected pvs
    QList<long long> getRemainingSpaceList();  // ditto
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
