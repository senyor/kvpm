/*
 *
 * 
 * Copyright (C) 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef VGCHANGE_H
#define VGCHANGE_H

#include <KDialog>
#include <QStringList>
#include <QRadioButton>
#include <QCheckBox>
#include <KComboBox>
#include <QSpinBox>
#include <QGroupBox>

class VolGroup;
bool change_vg(VolGroup *VolumeGroup);

class VGChangeDialog : public KDialog
{
Q_OBJECT

    VolGroup *m_vg;
    QString m_vg_name;
    QRadioButton *m_normal, *m_contiguous, *m_anywhere, *m_cling, 
                 *m_available_yes, *m_available_no, *m_polling_yes, *m_polling_no,
                 *m_limit_pv_yes, *m_limit_lv_yes, *m_limit_pv_no, *m_limit_lv_no;

    QCheckBox *m_resize, *m_clustered, *m_refresh, *m_uuid;
    KComboBox *m_extent_size_combo, *m_extent_suffix_combo;
    QGroupBox *m_limit_box, *m_lvlimit_box, *m_pvlimit_box, *m_available_box, *m_polling_box;
    QSpinBox  *m_max_lvs_spin, *m_max_pvs_spin; 

public:
    VGChangeDialog(VolGroup *volumeGroup, QWidget *parent = 0);
    QStringList args();

private slots:
    void limitExtentSize(int index);
    
};

#endif
