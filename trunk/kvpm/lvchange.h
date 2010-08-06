/*
 *
 * 
 * Copyright (C) 2008, 2010 Benjamin Scott   <benscott@nwlink.com>
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

#include <KDialog>
#include <KTabWidget>
#include <KLineEdit>
#include <KComboBox>

#include <QStringList>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>

class LogVol;

bool change_lv(LogVol *logicalVolumes);


class LVChangeDialog : public KDialog
{
Q_OBJECT

    LogVol *m_lv;
    
    QCheckBox *available_check,   // Make the volume available 
	      *ro_check,          // make the volume read only
	      *refresh_check,     // refresh the metadata
              *resync_check,      // re-sync mirrors
              *m_udevsync_check;  // sync with udev

    QRadioButton *m_normal_button, *m_contiguous_button, *m_anywhere_button, *m_cling_button; // allocation policy

    QRadioButton *m_monitor_button,  *m_nomonitor_button, *m_ignore_button, // dmeventd monitoring
                 *m_poll_button, *m_nopoll_button;

    QGroupBox *m_persistant_box, *m_dmeventd_box, *m_polling_box, *m_udevsync_box, *m_tag_group, *m_alloc_box;

    KLineEdit *minor_edit,  // User entered device minor number 
              *major_edit,  // User entered device major number 
              *m_tag_edit;  // new tag

    KComboBox *m_deltag_combo;

    QWidget *m_general_tab;
    QWidget *m_advanced_tab;
    QWidget *m_mirror_tab;

    void buildGeneralTab();
    void buildMirrorTab();
    void buildAdvancedTab();
    
public:
    LVChangeDialog(LogVol *logicalVolume, QWidget *parent = 0);
    QStringList arguments();
    
};

#endif
