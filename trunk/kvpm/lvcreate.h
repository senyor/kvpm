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
#ifndef LVCREATE_H
#define LVCREATE_H

#include <KDialog>
#include <KLineEdit>
#include <KTabWidget>
#include <KComboBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QList>
#include <QStringList>
#include <QRadioButton>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleValidator>

class LogVol;
class VolGroup;
class PhysVol;

bool LVCreate(VolGroup *VolumeGroup);
bool LVExtend(LogVol *LogicalVolume);
bool SnapshotCreate(LogVol *LogicalVolume);

class LVCreateDialog : public KDialog
{
Q_OBJECT
     bool snapshot;   // TRUE if a snapshot
     bool extend;     // TRUE if extending a volume
     
     VolGroup *vg;
     LogVol *lv;      // origin for snap or lv to extend
                      // set to NULL if creating a new logical volume

     QWidget *general_tab, *physical_tab, *advanced_tab;
     
     KLineEdit *minor_number_edit, *major_number_edit,
	       *name_edit, *size_edit;

     QCheckBox *zero_check, *readonly_check;
     QGroupBox *persistent_box, *stripe_box;
 
     KTabWidget *tab_widget;

     QList<QCheckBox *> pv_checks;        // these 2 lists are the same size and order
     QList<PhysVol *> physical_volumes;   // with each pv associated with a check box

     long long volume_size;     // proposed logical volume size in extents
     long long allocateable_space, allocateable_extents;
     KComboBox *size_combo, *stripe_size_combo; 
     QSpinBox *size_spin, *stripes_number_spin;
     QDoubleValidator *size_validator;
     QLabel *stripes_label, *stripes_count_label, *max_size_label, *max_extents_label,
            *allocateable_space_label, *allocateable_extents_label;
     
     QRadioButton *contiguous_button, *normal_button,   //Radio button to chose 
	          *anywhere_button, *inherited_button,  // the allocation policy
	          *cling_button;

     QWidget* createGeneralTab();
     QWidget* createAdvancedTab();
     QWidget* createPhysicalTab();
     long long getLargestVolume(int stripes); // largest lv possible with n stripes
     int getStripes();

 public:
     LVCreateDialog(VolGroup *Group, QWidget *parent = 0);
     LVCreateDialog(LogVol *LogicalVolume, bool Snapshot, QWidget *parent = 0);
     
     QStringList argumentsFS();
     QStringList argumentsLV();

 private slots:
     void adjustSizeCombo(int index);
     void setMaxSize(bool toggle_state);
     void setMaxSize(int stripes);
     void adjustSizeEdit(int percentage);
     void validateSizeEdit(QString size);
     long long convertSizeToExtents(int index, double size);
     void calculateSpace(bool checked);
     void zeroReadonlyCheck(int state);
};

#endif

