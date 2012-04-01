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

#ifndef PVCHANGE_H
#define PVCHANGE_H

#include <KComboBox>
#include <KDialog>
#include <KLineEdit>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QStringList>

class PhysVol;

class PVChangeDialog : public KDialog
{
    Q_OBJECT

    KLineEdit *m_tag_edit;      // new tag to add
    KComboBox *m_deltag_combo;  // old tag to remove
    QGroupBox *m_tags_group;
    QCheckBox *m_allocation_box;
    QCheckBox *m_uuid_box;
    QCheckBox *m_mda_box;
    PhysVol   *m_pv;

public:
    explicit PVChangeDialog(PhysVol *physicalVolume, QWidget *parent = 0);
    QStringList arguments();

private slots:
    void resetOkButton();

};

#endif
