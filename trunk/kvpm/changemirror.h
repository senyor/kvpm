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

#ifndef CHANGEMIRROR_H
#define CHANGEMIRROR_H

#include <KDialog>

#include <QStringList>

class KComboBox;
class KIcon;
class KIntSpinBox;
class KTabWidget;

class QGroupBox;
class QHBoxLayout;
class QLabel;
class QRadioButton;
class QStackedWidget;
class QVBoxLayout;

class LogVol;
class PvGroupBox;
class VolGroup;
class NoMungeCheck;
class NoMungeRadioButton;

class ChangeMirrorDialog : public KDialog
{
    Q_OBJECT

    KTabWidget  *m_tab_widget;
    KIntSpinBox *m_add_mirrors_spin;
    KIntSpinBox *m_stripe_spin;
    QStackedWidget *m_error_stack;
    PvGroupBox *m_pv_box;
    QGroupBox  *m_stripe_box;
    QGroupBox  *m_log_box;
    QWidget    *m_log_widget;
    KComboBox  *m_stripe_size_combo;
    KComboBox  *m_type_combo;

    QList<NoMungeCheck *> m_mirror_log_checks;

    bool m_bailout;
    bool m_change_log;    // true if we just changing the logging of an existing mirror
    LogVol *m_lv;         // The volume we are adding a mirror leg to.

    QRadioButton *m_core_log_button,
                 *m_mirrored_log_button,
                 *m_disk_log_button;

    // these are the pv names of the two mirror logs if we are changing the log 
    // count on a mirrored log 
    NoMungeRadioButton *m_log_one; 
    NoMungeRadioButton *m_log_two;

    QWidget *buildGeneralTab(const bool isRaidMirror, const bool isLvmMirror);
    QWidget *buildPhysicalTab(const bool isRaidMirror);
    QWidget *buildLogWidget();
    QStringList getPvsInUse();
    bool validateStripeSpin();
    void setLogRadioButtons();
    int getNewLogCount();

public:
    explicit ChangeMirrorDialog(LogVol *const mirrorVolume, bool changeLog, QWidget *parent = NULL);
    QStringList arguments();
    bool bailout();

private slots:
    void resetOkButton();
    void commitChanges();
    void enableTypeOptions(int index);
    void enableLogWidget();
};

#endif


