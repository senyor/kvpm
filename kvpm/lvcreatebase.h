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


#include <QStringList>

#include "kvpmdialog.h"
#include "misc.h"

class VolGroup;

class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QRegExpValidator;
class QStringList;
class QTabWidget;
class QVBoxLayout;

class SizeSelectorBox;


class LvCreateDialogBase : public KvpmDialog
{
    Q_OBJECT

    const VolGroup *const m_vg;

    QVBoxLayout *m_general_layout;

    bool m_use_si_units;  // true == Metric SI sizes = MB and GB, otherise use MiB, GiB etc.
    bool m_extend;        // true == we are extending a volume
    bool m_ispool;        // true == creating or extending a thin pool

    SizeSelectorBox *m_size_selector;

    QRegExpValidator *m_name_validator,
                     *m_tag_validator;

    QLineEdit *m_minor_edit, *m_major_edit,
              *m_name_edit,  *m_tag_edit;

    QCheckBox *m_readonly_check,
              *m_monitor_check,     // monitor with dmeventd
              *m_udevsync_check,    // sync operation with udev
              *m_zero_check,        // write zero at start of volume
              *m_skip_sync_check,   // skip initial sync of mirror
              *m_extend_fs_check;   // extend filesystem along with volume

    QGroupBox *m_persistent_box;
   
    QTabWidget *m_tab_widget;

    QLabel *m_max_size_label, *m_stripes_label, *m_maxfs_size_label, 
           *m_warning_label,  *m_current_label, *m_extend_label;

    long long m_max_size;    // only used for setting the labels.
    long long m_maxfs_size;  // can be retrieved and used by subclasses

    QWidget *m_warning_widget;
    QWidget *createWarningWidget();

    QWidget* createGeneralTab(const bool showNameTag, const bool showRo, const bool showZero, const bool showMisc);
    QWidget* createAdvancedTab(const bool showPersistent, const bool showSkipSync, const bool showMonitor);

signals:
    void extendFs();

private slots:
    void zeroReadOnlyEnable();

protected slots:
    virtual void resetOkButton() = 0;
    void setSizeLabels();

protected:
    const VolGroup *getVg();
    virtual QStringList args() = 0;
    virtual long long getLargestVolume() = 0;
    virtual bool isValid();    // Has a valid size, name, major number, minor number and tag
    bool isLow();    // size too small

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
    void setInfoLabels(const VolumeType type, const int stripes, const int mirrors, const long long maxSize);
    void setWarningLabel(const QString message);
    void clearWarningLabel();
    void setSelectorMaxExtents(const long long max);
    long long getSelectorExtents();
    long long getMaxFsSize();
    bool getExtendFs();    // is the extend fs check boxed checked
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
    LvCreateDialogBase(const VolGroup *const vg, const long long maxFsSize, 
                       const bool extend, const bool snap, const bool thin, const bool thinpool,
                       QString name = QString(""), QString pool = QString(""),  // name = origin for snap or lvname for extend 
                       QWidget *parent = NULL);

    virtual ~LvCreateDialogBase() {}     
};

#endif

