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


#ifndef PVMOVE_H
#define PVMOVE_H

#include <KDialog>
#include <QString>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QLabel>


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
    PvGroupBox *m_pv_checkbox;                   // many target pvs may be selected

    QRadioButton *m_contiguous_button, *m_normal_button,   // Radio button to chose
                 *m_anywhere_button, *m_inherited_button,  // the allocation policy
                 *m_cling_button;

    void buildDialog();
    void removeFullTargets();
    void setupSegmentMove(int segment);
    void setupFullMove();
    QStringList arguments();
    
public:
    explicit PVMoveDialog(PhysVol *physicalVolume, QWidget *parent = 0);
    explicit PVMoveDialog(LogVol *logicalVolume, int segment, QWidget *parent = 0);
    ~PVMoveDialog();
    bool bailout();    

private slots:
    void resetOkButton();
    void disableSource();
    void commitMove();

};

#endif
