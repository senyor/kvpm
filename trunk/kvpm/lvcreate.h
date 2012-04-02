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

#include <QList>
#include <QStringList>

class KIntSpinBox;
class KLineEdit;
class KTabWidget;
class KComboBox;

class QCheckBox;
class QDoubleValidator;
class QGroupBox;
class QLabel;
class QRadioButton;
class QRegExpValidator;

class LogVol;
class VolGroup;
class PhysVol;
class PvGroupBox;
class SizeSelectorBox;


class LVCreateDialog : public KDialog
{
    Q_OBJECT

    bool m_snapshot;        // TRUE if a snapshot
    bool m_extend;          // TRUE if extending a volume
    bool m_name_is_valid;   // TRUE if the new name is acceptable
    bool m_bailout;         // TRUE if we should not bother to execute this dialog
    bool m_fs_can_extend;
    bool m_use_si_units;    // TRUE Metric SI sizes = MB and GB, otherise use MiB, GiB etc.

    SizeSelectorBox *m_size_selector;

    VolGroup *m_vg;
    LogVol *m_lv;      // origin for snap or lv to extend
    // set to NULL if creating a new logical volume

    QWidget *m_general_tab,   // The tab labeled "general" in the dialog
            *m_physical_tab,  // The physical tab
            *m_advanced_tab;  // Adevanced options tab

    KLineEdit *m_minor_number_edit, *m_major_number_edit,
              *m_name_edit, *m_tag_edit;

    QRegExpValidator *m_name_validator,
                     *m_tag_validator;

    QCheckBox *m_zero_check,
              *m_readonly_check,
              *m_skip_sync_check,   // don't synchronize empty mirror images when created
              *m_monitor_check;     // monitor with dmeventd

    QGroupBox *m_persistent_box,
              *m_mirror_box,
              *m_stripe_box;

    PvGroupBox *m_pv_checkbox;

    KTabWidget *m_tab_widget;

    KComboBox *m_stripe_size_combo;

    KIntSpinBox *m_mirror_count_spin,  // how many mirrors we want
                *m_stripe_count_spin;  // how many stripes we want

    QLabel *m_stripe_count_label,
           *m_max_size_label,
           *m_max_extents_label,
           *m_extend_by_label,        // how much space we are adding to a volume
           *m_current_size_label;     // if we are extending this is the existing size

    QRadioButton *contiguous_button, *normal_button,     // Radio button to chose
                 *anywhere_button,   *inherited_button,  // the allocation policy
                 *cling_button;

    QRadioButton *m_mirrored_log, *m_disk_log, *m_core_log;

    QWidget* createGeneralTab();
    QWidget* createAdvancedTab();
    QWidget* createPhysicalTab();
    long long getLargestVolume();
    int getStripeCount();
    int getMirrorCount();
    void resetOkButton();
    void makeConnections();
    long long roundExtentsToStripes(long long extents);
    bool hasInitialErrors();
    QStringList argumentsLV();

public:
    explicit LVCreateDialog(VolGroup *const group, QWidget *parent = 0);
    LVCreateDialog(LogVol *const volume, const bool snapshot, QWidget *parent = 0);
    bool bailout();

private slots:
    void setMaxSize();
    void validateVolumeName(QString name);
    void zeroReadonlyCheck(int state);
    void enableMonitoring(bool checked);
    void commitChanges();
};

#endif

