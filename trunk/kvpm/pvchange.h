/*
 *
 *
 * Copyright (C) 2008, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef PVCHANGE_H
#define PVCHANGE_H


#include <QStringList>

#include "kvpmdialog.h"

class KComboBox;
class KLineEdit;

class QCheckBox;
class QGroupBox;
class QLabel;

class PhysVol;

class PVChangeDialog : public KvpmDialog
{
    Q_OBJECT

    KLineEdit *m_tag_edit;      // new tag to add
    KComboBox *m_deltag_combo;  // old tag to remove
    QGroupBox *m_tags_group;
    QCheckBox *m_allocation_box;
    QCheckBox *m_uuid_box;
    QCheckBox *m_mda_box;
    PhysVol   *m_pv;

    QStringList arguments();

public:
    explicit PVChangeDialog(PhysVol *physicalVolume, QWidget *parent = nullptr);

private slots:
    void commit();
    void resetOkButton();

};

#endif
