/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef LVCREATE_H
#define LVCREATE_H

#include <KDialog>

#include <QStringList>

#include "lvcreatebase.h"

class KIntSpinBox;
class KComboBox;
class QGroupBox;

class LogVol;
class PvGroupBox;


class LVCreateDialog : public LvCreateDialogBase
{
    Q_OBJECT

    bool m_ispool;          // TRUE if a thin pool
    bool m_snapshot;        // TRUE if a snapshot
    bool m_extend;          // TRUE if extending a volume
    bool m_bailout;         // TRUE if we should not bother to execute this dialog
    bool m_use_si_units;    // TRUE Metric SI sizes = MB and GB, otherise use MiB, GiB etc.

    LogVol *m_lv;      // origin for snap or lv to extend
                       // set to NULL if creating a new logical volume

    QWidget *m_physical_tab;  // The physical tab

    QGroupBox *m_volume_box;   
    PvGroupBox *m_pv_box;

    KComboBox *m_stripe_size_combo,
              *m_type_combo,
              *m_log_combo,
              *m_chunk_combo;

    KIntSpinBox *m_mirror_count_spin,  // how many mirrors we want
                *m_stripe_count_spin;  // how many stripes we want

    QWidget *m_stripe_widget,
            *m_mirror_widget;

    void buildDialog();
    QWidget* createPhysicalTab();
    QWidget* createTypeWidget(int pvcount);
    QWidget* createStripeWidget();
    QWidget* createChunkWidget();
    QWidget* createMirrorWidget(int pvcount);
    long long getLargestVolume();
    int getNeededStripes();
    int getLogCount();
    int getMaxStripes();
    int getChunkSize();
    int getChunkSize(long long const volumeSize);
    void makeConnections();
    void extendLastSegment(QList<long long> &committed, QList<long long> &available);
    long long roundExtentsToStripes(long long extents);
    bool hasInitialErrors();
    bool getPvsByPolicy(QList<long long> &usableBytes);
    QStringList args();

public:
    explicit LVCreateDialog(VolGroup *const vg, bool ispool, QWidget *parent = 0);
    LVCreateDialog(LogVol *const volume, const bool snapshot, QWidget *parent = 0);
    bool bailout();

private slots:
    void setMaxSize();
    void resetOkButton();
    void enableMonitoring(int index);
    void enableTypeOptions(int index);
    void enableStripeCombo(int value);
    void commit();
};

#endif

