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

#ifndef VGCREATE_H
#define VGCREATE_H

#include <KDialog>
#include <KLineEdit>
#include <KComboBox>

#include <QLabel>
#include <QProgressBar>
#include <QStringList>
#include <QCheckBox>
#include <QRegExpValidator>

#include "misc.h"

bool create_vg(QString physicalVolumePath, long long size);

bool create_vg();

class VGCreateDialog : public KDialog
{
Q_OBJECT

    QLabel *m_pv_label, *m_total_label;
    QList<NoMungeCheck *> m_pv_checks;
    QVBoxLayout *m_layout;

    KLineEdit *m_vg_name, 
              *m_max_lvs, 
              *m_max_pvs;

    QCheckBox *m_clustered, 
              *m_auto_backup, 
	      *m_max_lvs_check, 
              *m_max_pvs_check;

    KComboBox *m_extent_size, 
              *m_extent_suffix;

    QRegExpValidator *m_validator;
    QProgressBar *m_progress_bar;
    
 public:
    VGCreateDialog(QList<AvailableDevice *> devices, QWidget *parent = 0);
    
 private slots:
    void limitLogicalVolumes(int boxstate);
    void limitPhysicalVolumes(int boxstate);
    void validateOK();
    void commitChanges();
    void limitExtentSize(int);
    void selectAll();
    void selectNone();    
};

#endif
