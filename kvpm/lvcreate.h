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

bool LVCreate(VolGroup *volumeGroup);
bool LVExtend(LogVol *logicalVolume);
bool SnapshotCreate(LogVol *logicalVolume);

class LVCreateDialog : public KDialog
{
Q_OBJECT
     bool m_snapshot;   // TRUE if a snapshot
     bool m_extend;     // TRUE if extending a volume
     
     VolGroup *m_vg;
     LogVol *m_lv;      // origin for snap or lv to extend
                        // set to NULL if creating a new logical volume

     QWidget *m_general_tab,   // The tab labeled "general" in the dialog 
	     *m_physical_tab,  // The physical tab 
	     *m_advanced_tab;  // Adevanced options tab
     
     KLineEdit *m_minor_number_edit, 
	       *m_major_number_edit,
	       *m_name_edit, 
	       *m_size_edit;

     QCheckBox *m_zero_check, 
               *m_readonly_check;

     QGroupBox *m_persistent_box,
	       *m_mirror_box,
	       *m_stripe_box;
 
     KTabWidget *m_tab_widget;

     QList<QCheckBox *> m_pv_checks;        // these 2 lists are the same size and order
     QList<PhysVol *> m_physical_volumes;   // with each pv associated with a check box

     long long m_volume_extents,               // proposed logical volume size in extents
	       m_allocateable_space, 
	       m_allocateable_extents;

     KComboBox *size_combo, *stripe_size_combo; 

     QSpinBox *m_size_spin,
	      *m_mirrors_number_spin,  // how many mirrors we want
	      *m_stripes_number_spin;  // how many stripes we want

     QDoubleValidator *m_size_validator;

     QLabel *m_stripes_count_label,
	    *m_max_size_label, 
            *m_max_extents_label,
            *m_allocateable_space_label, 
	    *m_allocateable_extents_label;
     
     QRadioButton *contiguous_button, *normal_button,   //Radio button to chose 
	          *anywhere_button, *inherited_button,  // the allocation policy
	          *cling_button;

     QRadioButton *m_disk_log,      // if this is a mirror, which kind of log
	          *m_core_log;      // do we want
     
     QWidget* createGeneralTab();
     QWidget* createAdvancedTab();
     QWidget* createPhysicalTab();
     long long getLargestVolume(int stripes); 
     long long getLargestMirror(int mirrors, bool disk_log);
     int getStripeCount();
     int getMirrorCount();
     void resetOkButton();

 public:
     LVCreateDialog(VolGroup *volumeGroup, QWidget *parent = 0);
     LVCreateDialog(LogVol *logicalVolume, bool snapshot, QWidget *parent = 0);
     
     QStringList argumentsFS();
     QStringList argumentsLV();

 private slots:
     void adjustSizeCombo(int index);
     void enableStripeBox(bool toggle_state);
     void enableMirrorBox(bool toggle_state);
     void setMaxSize(bool toggle_state);
     void setMaxSize(int stripes);
     void adjustSizeEdit(int percentage);
     void validateVolumeSize(QString size);
     long long convertSizeToExtents(int index, double size);
     void calculateSpace(bool checked);
     void zeroReadonlyCheck(int state);
};

#endif

