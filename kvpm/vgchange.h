/*
 *
 *
 * Copyright (C) 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
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

#include "kvpmdialog.h"

class KComboBox;
class KIntSpinBox;

class QRadioButton;
class QCheckBox;
class QGroupBox;

class VolGroup;
class PolicyComboBox;

class VGChangeDialog : public KvpmDialog
{
    Q_OBJECT

    VolGroup *m_vg;
    QRadioButton *m_available_yes, *m_available_no, *m_polling_yes, *m_polling_no;
    QCheckBox *m_resize, *m_clustered, *m_refresh, *m_uuid;
    KComboBox *m_extent_size_combo, *m_extent_suffix_combo;
    QGroupBox *m_limit_box, *m_lvlimit_box, *m_pvlimit_box, *m_available_box, *m_polling_box;
    KIntSpinBox  *m_max_lvs_spin, *m_max_pvs_spin;
    PolicyComboBox *m_policy_combo;

    QStringList arguments();

public:
    explicit VGChangeDialog(VolGroup *const group, QWidget *parent = nullptr);

private slots:
    void limitExtentSize(const int index);
    void resetOkButton();
    void commit();

};

#endif
