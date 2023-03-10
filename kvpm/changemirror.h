/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013, 2016 Benjamin Scott   <benscott@nwlink.com>
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


#include <QSharedPointer>
#include <QStringList>

#include "kvpmdialog.h"

class QComboBox;
class QGroupBox;
class QRadioButton;
class QSpinBox;
class QStackedWidget;
class QTabWidget;

class LogVol;
class NoMungeCheck;
class NoMungeRadioButton;
class PvGroupBox;
class PvSpace;


class ChangeMirrorDialog : public KvpmDialog
{
    Q_OBJECT

    QTabWidget  *m_tab_widget = nullptr;
    QSpinBox *m_add_mirrors_spin = nullptr;
    QSpinBox *m_stripe_spin = nullptr;
    QStackedWidget *m_error_stack = nullptr;
    QStackedWidget *m_log_stack = nullptr;
    PvGroupBox *m_pv_box = nullptr;
    QGroupBox  *m_stripe_box = nullptr;
    QGroupBox  *m_log_box = nullptr;
    QWidget    *m_log_widget = nullptr;
    QComboBox  *m_stripe_size_combo = nullptr;
    QComboBox  *m_type_combo = nullptr;

    QStringList m_log_pvs;
    QStringList m_image_pvs;
    QList<QSharedPointer<PvSpace>> m_space_list; 
    QList<NoMungeCheck *> m_mirror_log_checks;

    bool m_change_log;    // true if we just changing the logging of an existing mirror
    LogVol *m_lv;   // The volume we are adding a mirror leg to.

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

    QStringList getLogPvs();
    QStringList getImagePvs();
    QList<QSharedPointer<PvSpace>> getPvSpaceList();
    bool pvHasLog(QString const pv);
    bool pvHasImage(QString const pv);
    bool validateStripeSpin();
    void setLogRadioButtons();
    int getNewLogCount();
    long long getNewLogSize();
    bool getAvailableByteList(QList<long long> &byte_list, int &unhandledLogs, const int stripes);

public:
    ChangeMirrorDialog(LogVol *const mirrorVolume, const bool changeLog, QWidget *parent = nullptr);
    QStringList arguments();
                           
private slots:
    void resetOkButton();
    void commit();
    void enableTypeOptions(int index);
    void enableLogWidget();
};

#endif


