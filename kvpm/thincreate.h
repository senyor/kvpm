/*
 *
 *
 * Copyright (C) 2012 Benjamin Scott   <benscott@nwlink.com>
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

#include <KDialog>

#include <QList>
#include <QStringList>

class KLineEdit;
class KTabWidget;
class KComboBox;

class QCheckBox;
class QGroupBox;
class QLabel;
class QRadioButton;
class QRegExpValidator;

class LogVol;
class VolGroup;
class SizeSelectorBox;


class ThinCreateDialog : public KDialog
{
    Q_OBJECT

    bool m_snapshot;        // TRUE if a snapshot
    bool m_extend;          // TRUE if extending a volume
    bool m_bailout;         // TRUE if we should not bother to execute this dialog
    bool m_fs_can_extend;
    bool m_use_si_units;    // TRUE Metric SI sizes = MB and GB, otherise use MiB, GiB etc.

    SizeSelectorBox *m_size_selector;

    VolGroup *m_vg;
    LogVol *m_lv;      // origin for snap or lv to extend
                       // set to NULL if creating a new logical volume
    LogVol *m_pool;    // The thin pool we are using if creating a new volume, set to NULL otherwise.

    QWidget *m_general_tab,   // The tab labeled "general" in the dialog
            *m_advanced_tab;  // Adevanced options tab

    KLineEdit *m_minor_edit, *m_major_edit,
              *m_name_edit,  *m_tag_edit;

    QRegExpValidator *m_name_validator,
                     *m_tag_validator;

    QCheckBox *m_readonly_check,
              *m_monitor_check,     // monitor with dmeventd
              *m_udevsync_check;    // sync operation with udev

    QGroupBox *m_persistent_box, *m_volume_box;
   
    KTabWidget *m_tab_widget;

    void buildDialog();
    QWidget* createGeneralTab();
    QWidget* createAdvancedTab();
    long long getLargestVolume();
    void makeConnections();
    bool hasInitialErrors();
    QStringList arguments();

public:
    explicit ThinCreateDialog(LogVol *const pool, QWidget *parent = 0);
    ThinCreateDialog(LogVol *const volume, const bool snapshot, QWidget *parent = 0);
    bool bailout();

private slots:
    void resetOkButton();
    void commitChanges();
};

#endif

