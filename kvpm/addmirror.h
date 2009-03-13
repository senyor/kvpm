/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef ADDMIRROR_H
#define ADDMIRROR_H

#include <KTabWidget>

#include <KDialog>
#include <QStringList>
#include <QLabel>
#include <QSpinBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QVBoxLayout>

class KIntSpinBox;

class LogVol;
class NoMungeCheck;
class VolGroup;

bool add_mirror(LogVol *logicalVolume);

/* AddMirror is also used to change mirror properties,
   such as logging, on existing mirrors */


class AddMirrorDialog : public KDialog
{
Q_OBJECT

    KTabWidget *m_tab_widget;
    KIntSpinBox *m_add_mirrors_spin;

    QVBoxLayout *m_general_layout;
    QVBoxLayout *m_physical_layout;

    QGroupBox *m_mirror_group;
    QGroupBox *m_add_mirror_box;

    QString m_logical_volume_name;
    LogVol *m_lv;                      // The volume we are adding a mirror to. 
    
    int m_current_leg_count;           // How many mirror legs do we already have?
                                       // An unmirrored volume counts as one.
    
    QRadioButton *contiguous_button, *normal_button,   //Radio button to chose 
	         *anywhere_button, *inherited_button,  // the allocation policy
	         *cling_button;

    QRadioButton *core_log, 
                 *disk_log;

    bool m_mirror_has_log;             // True if the volume is a mirror with an
                                       // existing mirror log.
    
    QList<NoMungeCheck *> m_pv_leg_checks;
    QList<long long> m_pv_leg_size;

    void setupGeneralTab();
    void setupPhysicalTab();
    QStringList getPvsInUse();
    
public:
    AddMirrorDialog(LogVol *logicalVolume, QWidget *parent = 0);
    QStringList arguments();

private slots:

    void comparePvsNeededPvsAvailable(int);
    void comparePvsNeededPvsAvailable(bool);
    void comparePvsNeededPvsAvailable();
    void clearCheckedPvs(bool checked);
    
};

#endif


