/*
 *
 * 
 * Copyright (C) 2008, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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

#include <KComboBox>
#include <KDialog>
#include <KLineEdit>

#include <QLabel>
#include <QStringList>
#include <QCheckBox>
#include <QRegExpValidator>
#include <QVBoxLayout>

class StorageDevice;
class StoragePartition;
class PvGroupBox;


class VGCreateDialog : public KDialog
{
Q_OBJECT

    bool m_bailout;

    QLabel *m_pv_label, 
           *m_total_available_label, 
           *m_total_selected_label;

    PvGroupBox  *m_pv_checkbox;

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

    void buildDialog(QList<StorageDevice *> devices, QList<StoragePartition *> partitions);
    void getUsablePvs(QList<StorageDevice *> &devices, QList<StoragePartition *> &partitions);
    void limitExtentSize(int);
    
 public:
    explicit VGCreateDialog(QWidget *parent = NULL);
    VGCreateDialog(StorageDevice *const device, StoragePartition *const partition, QWidget *parent = NULL);
    bool bailout();    

 private slots:
    //void limitLogicalVolumes(int boxstate);
    //void limitPhysicalVolumes(int boxstate);
    void validateOK();
    void commitChanges();
    void extentSizeChanged();

};

#endif
