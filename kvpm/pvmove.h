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


class PVMoveDialog : public KDialog
{
    Q_OBJECT

    VolGroup *m_vg;
    LogVol   *m_lv;
    bool      m_move_lv;
    bool      m_move_segment;
    bool      m_bailout;       // if TRUE, a move is impossible so don't even call up the dialog
    long long m_pv_used_space;

    QList<NameAndRange *> m_sources;
    QList<PhysVol *> m_target_pvs;               // destination physical volumes
    QList<NoMungeRadioButton *> m_radio_buttons; // user can select only one source pv
    PvGroupBox *m_pv_box;                        // many target pvs may be selected
    QLabel *m_radio_label = nullptr;   // number of extents selected for the pvmove source

    void buildDialog();
    void removeFullTargets();
    void setupSegmentMove(int segment);
    void setupFullMove();
    bool hasMovableExtents();
    QStringList arguments(); 
    QStringList getLvNames(); 
    QWidget* singleSourceWidget();
    bool isMovable(LogVol *lv);
    long long movableExtents();

public:
    explicit PVMoveDialog(PhysVol *const physicalVolume, QWidget *parent = nullptr);
    explicit PVMoveDialog(LogVol *const logicalVolume, int const segment, QWidget *parent = nullptr);
    ~PVMoveDialog();
    bool bailout();

private slots:
    void commitMove();
    void disableSource();
    void resetOkButton();
    void setRadioExtents();
};

#endif
