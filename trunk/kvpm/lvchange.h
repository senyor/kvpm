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
#include <QStringList>
#include <QGroupBox>
#include <QCheckBox>
#include <QLineEdit>

class LogVol;

bool change_lv(LogVol *LogicalVolumes);

class LVChangeGeneralTab : public QWidget
{
 public:
    LVChangeGeneralTab(LogVol *lv, QWidget *parent = 0);
    QCheckBox *available_check, *contig_check;
    QCheckBox *ro_check, *refresh_check;
    
};

class LVChangeAdvancedTab : public QWidget
{
 public:
    LVChangeAdvancedTab(LogVol *lv, QWidget *parent = 0);
    QGroupBox *persistant_box, *mirror_box;
    QLineEdit *minor_edit, *major_edit;
    QCheckBox *resync_check, *monitor_check;

};

class LVChangeDialog : public KDialog
{
Q_OBJECT
    KTabWidget *tab_widget;
    QGroupBox *group_box;
    QString lv_full_name;
    LVChangeGeneralTab *general_tab;
    LVChangeAdvancedTab *advanced_tab;
    
public:
    LVChangeDialog(LogVol *LogicalVolume, QWidget *parent = 0);
    QStringList arguments();
    
};

#endif
