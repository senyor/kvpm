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
    SizeSelectorBox *m_size_selector;

    //
    // DO THESE NEED TO BE GLOBAL !!

    QRegExpValidator *m_name_validator,
                     *m_tag_validator;

    //
    //


    KLineEdit *m_minor_edit, *m_major_edit,
              *m_name_edit,  *m_tag_edit;

    QCheckBox *m_readonly_check,
              *m_monitor_check,     // monitor with dmeventd
              *m_udevsync_check,    // sync operation with udev
              *m_zero_check,        // write zero at start of volume
              *m_skip_sync_check;   // skip initial sync of mirror

    QGroupBox *m_persistent_box, *m_volume_box;
   
    KTabWidget *m_tab_widget;

    QWidget* createGeneralTab(const bool showNameTag, const bool showRo, const bool showZero, const bool showMisc);
    QWidget* createAdvancedTab(const bool showPersistent, const bool showSkipSync, const bool showMonitor);
    void makeConnections();

protected slots:
    virtual void commit() = 0;
    virtual void resetOkButton() = 0;

protected:
    virtual QStringList args() = 0;
    virtual long long getLargestVolume() = 0;

    void setSkipSync(const bool skip);
    void setReadOnly(const bool ro);
    void setZero(const bool zero);
    void setMonitor(const bool monitor);
    void setUdev(const bool udev);
    void initializeSizeSelector(const long long extentSize, const long long currentSize, const long long maxSize);
    void setMaxExtents(const long long max);

    void enableMonitor(const bool monitor);
    void enableSkipSync(const bool skip);
    void enableReadOnly(const bool ro);
    void enableZero(const bool zero);

    // Has a valid size, name, major number, minor number and tag
    virtual bool isValid();

    bool getMonitor();
    bool getUdev();
    bool getReadOnly();
    bool getZero();
    bool getPersistent();  // is the persistent major, minor number check box checked
    QString getMajor();
    QString getMinor();
    QString getName();
    QString getTag();
    long long getSize();

public:
    LvCreateDialogBase(bool extend, bool snap, bool thin,
                       QString name = QString(""), QString pool = QString(""),  // name = origin for snap or lvname for extend 
                       QWidget *parent = NULL);

};

#endif

