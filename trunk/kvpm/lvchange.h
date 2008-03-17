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

#ifndef LVCHANGE_H
#define LVCHANGE_H

#include <KDialog>
#include <KTabWidget>
#include <KLineEdit>

#include <QStringList>
#include <QGroupBox>
#include <QCheckBox>

class LogVol;

bool change_lv(LogVol *logicalVolumes);


class LVChangeDialog : public KDialog
{
Q_OBJECT

    LogVol *m_lv;
    
    QCheckBox *available_check,   // Make the volume available 
	      *contig_check,      // allocate contiguous extents
	      *ro_check,          // make the volume read only
	      *refresh_check,     // refresh the metadata
	      *resync_check,      // re-sync mirrors
	      *monitor_check;     // dmeventd monitoring

    QGroupBox *m_persistant_box, *m_mirror_box;

    KLineEdit *minor_edit,  // User entered device minor number 
              *major_edit;  // User entered device major number 

    QWidget *m_general_tab;
    QWidget *m_advanced_tab;
    
    void buildGeneralTab();
    void buildAdvancedTab();
    
public:
    LVChangeDialog(LogVol *logicalVolume, QWidget *parent = 0);
    QStringList arguments();
    
};

#endif
