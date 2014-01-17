/*
 *
 *
 * Copyright (C) 2008, 2011, 2012, 2013, 2014 Benjamin Scott   <benscott@nwlink.com>
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

#include <QList>

#include "kvpmdialog.h"

class KLineEdit;
class KComboBox;

class QLabel;
class QCheckBox;
class QRegExpValidator;
class QVBoxLayout;

class StorageBase;
class PvGroupBox;


class VGCreateDialog : public KvpmDialog
{
    Q_OBJECT

    QLabel *m_pv_label,
           *m_total_available_label,
           *m_total_selected_label;

    PvGroupBox  *m_pv_checkbox = nullptr;

    KLineEdit *m_vg_name,
              *m_max_lvs,
              *m_max_pvs;

    QCheckBox *m_clustered,
              *m_auto_backup,
              *m_max_lvs_check,
              *m_max_pvs_check;

    KComboBox *m_extent_size,
              *m_extent_suffix;

    QRegExpValidator *m_validator = nullptr;

    void buildDialog(QList<StorageBase *> devices);
    void limitExtentSize(int);
    bool continueWarning();

public:
    explicit VGCreateDialog(QWidget *parent = nullptr);
    explicit VGCreateDialog(StorageBase *const device, QWidget *parent = nullptr);

private slots:
    //void limitLogicalVolumes(int boxstate);
    //void limitPhysicalVolumes(int boxstate);
    void validateOK();
    void commit();
    void extentSizeChanged();

};

#endif
