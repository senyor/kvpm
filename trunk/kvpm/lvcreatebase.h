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

#ifndef LVCREATEBASE_H
#define LVCREATEBASE_H

#include <KDialog>

#include <QList>
#include <QStringList>

#include "misc.h"

class KLineEdit;
class KTabWidget;
class KComboBox;

class QCheckBox;
class QGroupBox;
class QLabel;
class QRadioButton;
class QRegExpValidator;
class QStringList;
class QVBoxLayout;

class SizeSelectorBox;


class LvCreateDialogBase : public KDialog
{
    Q_OBJECT

    QVBoxLayout *m_general_layout;

    bool m_use_si_units;    // true == Metric SI sizes = MB and GB, otherise use MiB, GiB etc.
    bool m_extend;          // true == we are extending a volume
    bool m_ispool;        // true == creating or extending a thin pool

    SizeSelectorBox *m_size_selector;

    QRegExpValidator *m_name_validator,
                     *m_tag_validator;

    KLineEdit *m_minor_edit, *m_major_edit,
              *m_name_edit,  *m_tag_edit;

    QCheckBox *m_readonly_check,
              *m_monitor_check,     // monitor with dmeventd
              *m_udevsync_check,    // sync operation with udev
              *m_zero_check,        // write zero at start of volume
              *m_skip_sync_check;   // skip initial sync of mirror

    QGroupBox *m_persistent_box;
   
    KTabWidget *m_tab_widget;

    QLabel *m_maxextents_label, *m_stripes_label, *m_maxsize_label;

    QWidget* createGeneralTab(const bool showNameTag, const bool showRo, const bool showZero, const bool showMisc);
    QWidget* createAdvancedTab(const bool showPersistent, const bool showSkipSync, const bool showMonitor);


private slots:
    void zeroReadOnlyEnable();

protected slots:
    virtual void commit() = 0;
    virtual void resetOkButton() = 0;

protected:
    virtual QStringList args() = 0;
    virtual long long getLargestVolume() = 0;
    virtual bool isValid();    // Has a valid size, name, major number, minor number and tag

    void setSkipSync(const bool skip);
    void setReadOnly(const bool ro);
    void setZero(const bool zero);
    void setMonitor(const bool monitor);
    void setUdev(const bool udev);
    void initializeSizeSelector(const long long extentSize, const long long currentSize, const long long maxSize);
    void setPhysicalTab(QWidget *const tab);

    void enableMonitor(const bool monitor);
    void enableSkipSync(const bool skip);
    void enableReadOnly(const bool ro);
    void enableZero(const bool zero);
    void setInfoLabels(VolumeType type, int stripes, int mirrors, long long maxextents, long long maxsize);
    void setSelectorMaxExtents(const long long max);
    long long getSelectorExtents();

    bool getMonitor();
    bool getUdev();
    bool getReadOnly();
    bool getSkipSync();
    bool getZero();
    bool getPersistent();  // is the persistent major, minor number check box checked
    QString getMajor();
    QString getMinor();
    QString getName();
    QString getTag();

public:
    LvCreateDialogBase(const bool extend, const bool snap, const bool thin, const bool thinpool,
                       QString name = QString(""), QString pool = QString(""),  // name = origin for snap or lvname for extend 
                       QWidget *parent = NULL);

};

#endif

