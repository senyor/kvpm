/*
 *
 *
 * Copyright (C) 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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

#include <stdint.h>

#include <KComboBox>

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QStringList>
#include <QVariant>

#include "misc.h"

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
    QList<long long> m_normal;
    QList<long long> m_contiguous;

    QLabel *m_space_label, *m_extents_label;
    uint64_t m_extent_size;
    QHBoxLayout *getButtons();
    KComboBox *m_alloc_combo;

    void addLabelsAndButtons(QGridLayout *const layout, const int pvCount, AllocationPolicy const policy);
    QWidget *buildPolicyBox(AllocationPolicy const policy);

public:
    // Note: policy needs to be passed "inherited" here for new logical volumes
    explicit PvGroupBox(QList<PhysVol *> volumes, 
                        QList<long long> normal = QList<long long>(), 
                        QList<long long> contiguous = QList<long long>(), 
                        AllocationPolicy const policy = NO_POLICY, 
                        bool const target = false, 
                        QWidget *parent = NULL);

    PvGroupBox(QList<StorageDevice *> devices, 
               QList<StoragePartition *> partitions,
               uint64_t extentSize, 
               QWidget *parent = NULL);

    QStringList getAllNames();    // names of all pvs displayed in the box
    QStringList getNames();       // names of *selected* pvs
    long long getRemainingSpace();   // total unused space on selected pvs
    QList<long long> getRemainingSpaceList();  // ditto
    void setExtentSize(uint64_t extentSize);
    void disableOrigin(PhysVol *originVolume); // disable origin to prevent move from and to same pv
    AllocationPolicy getPolicy();

signals:
    void stateChanged();

private slots:
    void calculateSpace();
    void setChecksToPolicy();

public slots:
    void selectAll();
    void selectNone();

};

#endif
