/*
 *
 * 
 * Copyright (C) 2008, 2010 Benjamin Scott   <benscott@nwlink.com>
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
#include <QHBoxLayout>

class KIntSpinBox;
class KComboBox;
class LogVol;
class NoMungeCheck;
class PVCheckBox;
class VolGroup;

bool add_mirror(LogVol *logicalVolume);

/* AddMirror is also used to change mirror properties,
   such as logging, on existing mirrors */


class AddMirrorDialog : public KDialog
{
Q_OBJECT

    KTabWidget *m_tab_widget;
    KIntSpinBox *m_add_mirrors_spin;
    KIntSpinBox *m_stripes_number_spin;
    QHBoxLayout *m_general_layout;
    QVBoxLayout *m_physical_layout;

    PVCheckBox *m_pv_box;
    QGroupBox  *m_add_mirror_box;
    QGroupBox  *m_stripe_box;
    KComboBox  *m_stripe_size_combo;

    LogVol *m_lv;                      // The volume we are adding a mirror to. 
    
    int m_current_leg_count;           // How many mirror legs do we already have?
                                       // An unmirrored volume counts as one.
    
    QRadioButton *contiguous_button, *normal_button,   //Radio button to chose 
	         *anywhere_button, *inherited_button,  // the allocation policy
	         *cling_button;

    QRadioButton *m_core_log_button, *m_mirrored_log_button, *m_disk_log_button;

    //    QList<NoMungeCheck *> m_pv_leg_checks;
    //    QList<long long> m_pv_leg_size;

    void setupGeneralTab();
    void setupPhysicalTab();
    QStringList getPvsInUse();
    
public:
    AddMirrorDialog(LogVol *logicalVolume, QWidget *parent = 0);
    QStringList arguments();

private slots:
    void comparePvsNeededPvsAvailable();
    
};

#endif


