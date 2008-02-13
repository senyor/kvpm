/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Klvm project.
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
#include <QStringList>
#include <QCheckBox>

bool create_vg(QString PhysicalVolumePath);


class VGCreateDialog : public KDialog
{
Q_OBJECT
    QString pv_path;
    KLineEdit *vg_name, *max_lvs, *max_pvs;
    QCheckBox *clustered, *auto_backup, *max_lvs_check, *max_pvs_check;
    KComboBox *extent_size, *extent_suffix;
    
 public:
    VGCreateDialog(QString PhysicalVolumePath, QWidget *parent = 0);
    QStringList arguments();
    
 private slots:
    void limitLogicalVolumes(int boxstate);
    void limitPhysicalVolumes(int boxstate);
 
};

#endif
