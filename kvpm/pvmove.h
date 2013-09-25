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


#ifndef PVMOVE_H
#define PVMOVE_H

#include "kvpmdialog.h"

#include <KDialog>

#include <QList>

class QCheckBox;
class QGroupBox;
class QLabel;
class QRadioButton;
class QString;

class LogVol;
class NameAndRange;
class NoMungeRadioButton;
class PhysVol;
class PvGroupBox;
class VolGroup;

bool restart_pvmove();
bool stop_pvmove();


class PVMoveDialog : public KvpmDialog
{
    Q_OBJECT

    VolGroup *m_vg = nullptr;
    LogVol   *m_lv = nullptr;
    int m_segment;
    bool      m_move_segment;
    long long m_pv_used_space;

    QList<NameAndRange *> m_sources;
    QList<NoMungeRadioButton *> m_radio_buttons; // user can select only one source pv
    PvGroupBox *m_pv_box = nullptr;              // many target pvs may be selected
    QLabel *m_radio_label = nullptr;   // number of extents selected for the pvmove source

    void buildDialog(QList<PhysVol *> targets);
    void setupSegmentMove(int segment);
    void setupFullMove();
    bool hasMovableExtents();
    QStringList arguments(); 
    QStringList getLvNames(); 
    QList<PhysVol *> removeForbiddenTargets(QList<PhysVol *> targets, const QString source);
    QList<PhysVol *> removeFullTargets(QList<PhysVol *> targets);
    QStringList getForbiddenTargets(LogVol *const lv, const QString source); 
    QWidget* singleSourceWidget();
    QWidget* bannerWidget();
    bool isMovable(LogVol *lv);
    long long movableExtents();

public:
    explicit PVMoveDialog(PhysVol *const physicalVolume, QWidget *parent = nullptr);
    explicit PVMoveDialog(LogVol *const logicalVolume, int const segment = -1, QWidget *parent = nullptr);
    ~PVMoveDialog();

private slots:
    void commit();
    void disableTargets();
    void resetOkButton();
    void setRadioExtents();
};

#endif
