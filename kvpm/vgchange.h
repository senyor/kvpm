/*
 *
 *
 * Copyright (C) 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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

class KComboBox;
class KIntSpinBox;

class QRadioButton;
class QCheckBox;
class QGroupBox;

class VolGroup;

class VGChangeDialog : public KDialog
{
    Q_OBJECT

    VolGroup *m_vg;
    QString m_vg_name;
    QRadioButton *m_normal, *m_contiguous, *m_anywhere, *m_cling,
                 *m_available_yes, *m_available_no, *m_polling_yes, *m_polling_no;

    QCheckBox *m_resize, *m_clustered, *m_refresh, *m_uuid;
    KComboBox *m_extent_size_combo, *m_extent_suffix_combo;
    QGroupBox *m_limit_box, *m_lvlimit_box, *m_pvlimit_box, *m_available_box, *m_polling_box;
    KIntSpinBox  *m_max_lvs_spin, *m_max_pvs_spin;
    QStringList arguments();

public:
    explicit VGChangeDialog(VolGroup *const volumeGroup, QWidget *parent = 0);
    bool bailout();

private slots:
    void limitExtentSize(int index);
    void resetOkButton();
    void commitChanges();

};

#endif
