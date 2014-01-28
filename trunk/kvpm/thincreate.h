/*
 *
 *
 * Copyright (C) 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef THINCREATE_H
#define THINCREATE_H

#include "lvcreatebase.h"

#include <QStringList>


class LogVol;



class ThinCreateDialog : public LvCreateDialogBase
{
    Q_OBJECT

    bool m_snapshot;        // TRUE if a snapshot
    bool m_extend;          // TRUE if extending a volume
    bool m_fs_can_extend;
    bool m_use_si_units;    // TRUE Metric SI sizes = MB and GB, otherise use MiB, GiB etc.


    LogVol *m_lv;      // origin for snap or lv to extend
                       // set to NULL if creating a new logical volume
    LogVol *m_pool;    // The thin pool we are using if creating a new volume, set to NULL otherwise.

    long long getLargestVolume();
    bool hasInitialErrors();
    QStringList args();

public:
    explicit ThinCreateDialog(LogVol *const pool, QWidget *parent = nullptr);
    ThinCreateDialog(LogVol *const volume, const bool snapshot, QWidget *parent = nullptr);

private slots:
    void setMaxSize();
    void resetOkButton();
    void commit();
};

#endif

