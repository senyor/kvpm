/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef LVCHANGE_H
#define LVCHANGE_H

#include <QStringList>

#include "kvpmdialog.h"

class KComboBox;
class KLineEdit;
class KTabWidget;

class QCheckBox;
class QGroupBox;
class QRadioButton;

class LogVol;
class PolicyComboBox;


class LVChangeDialog : public KvpmDialog
{
    Q_OBJECT

    LogVol *m_lv;

    PolicyComboBox *m_policy_combo;

    QCheckBox *m_available_check,    // Make the volume available
              *m_ro_check,           // make the volume read only
              *m_refresh_check,      // refresh the metadata
              *m_udevsync_check,     // sync with udev
              *m_persistent_check;   // Set persistent kernel device numbers

    QRadioButton *m_poll_button,
                 *m_nopoll_button,
                 *m_monitor_button,    // dmeventd monitoring
                 *m_nomonitor_button,
                 *m_ignore_button;     // ignore dmeventd monitoring

    QGroupBox *m_devnum_box,
              *m_dmeventd_box,
              *m_polling_box;

    KLineEdit *m_minor_edit,  // User entered device minor number
              *m_major_edit,  // User entered device major number
              *m_tag_edit;    // new tag

    KComboBox *m_deltag_combo;

    QWidget *buildGeneralTab();
    QWidget *buildAdvancedTab();
    QStringList arguments();

public:
    explicit LVChangeDialog(LogVol *const volume, QWidget *parent = nullptr);

private slots:
    void commit();
    void resetOkButton();
    void refreshAndAvailableCheck();

};

#endif
