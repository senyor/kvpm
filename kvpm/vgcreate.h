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

#include <QStringList>
#include <QCheckBox>
#include <QRegExpValidator>


bool create_vg(QString physicalVolumePath);


class VGCreateDialog : public KDialog
{
Q_OBJECT

    QString   m_pv_path;

    KLineEdit *m_vg_name, *m_max_lvs, *m_max_pvs;

    QCheckBox *m_clustered, *m_auto_backup, 
	      *m_max_lvs_check, *m_max_pvs_check;

    KComboBox *m_extent_size, *m_extent_suffix;

    QRegExpValidator *m_validator;
    
 public:
    VGCreateDialog(QString physicalVolumePath, QWidget *parent = 0);
    QStringList arguments();
    
 private slots:
    void limitLogicalVolumes(int boxstate);
    void limitPhysicalVolumes(int boxstate);
    void validateName(QString name);
    
};

#endif
