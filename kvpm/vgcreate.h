/*
 *
 * 
 * Copyright (C) 2008, 2011 Benjamin Scott   <benscott@nwlink.com>
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
#include <QStringList>
#include <QCheckBox>
#include <QRegExpValidator>
#include <QVBoxLayout>

class StorageDevice;
class StoragePartition;
class PVCheckBox;

bool create_vg(StorageDevice *device, StoragePartition *partition);

bool create_vg();

class VGCreateDialog : public KDialog
{
Q_OBJECT

    QLabel *m_pv_label, 
           *m_total_available_label, 
           *m_total_selected_label;

    PVCheckBox  *m_pv_checkbox;
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
    //    QProgressBar *m_progress_bar;

    void limitExtentSize(int);
    
 public:
    VGCreateDialog(QList<StorageDevice *> devices, QList<StoragePartition *> partitions, QWidget *parent = 0);
    
 private slots:
    //void limitLogicalVolumes(int boxstate);
    //void limitPhysicalVolumes(int boxstate);
    void validateOK();
    void commitChanges();
    void extentSizeChanged();

};

#endif
