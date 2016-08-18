/*
 *
 *
 * Copyright (C) 2008, 2011, 2012, 2013, 2014, 2016 Benjamin Scott   <benscott@nwlink.com>
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


#include <QList>

#include "kvpmdialog.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QCheckBox;
class QRegExpValidator;

class StorageBase;
class PvGroupBox;


class VGCreateDialog : public KvpmDialog
{
    Q_OBJECT

    QLabel *m_pv_label,
           *m_total_available_label,
           *m_total_selected_label;

    PvGroupBox  *m_pv_checkbox = nullptr;

    QLineEdit *m_vg_name,
              *m_max_lvs,
              *m_max_pvs,
              *m_size_edit,
              *m_align_edit,
              *m_offset_edit;
    
    QCheckBox *m_clustered,
              *m_auto_backup,
              *m_max_lvs_check,
              *m_max_pvs_check;

    QComboBox *m_extent_size,
              *m_extent_suffix,
              *m_copies_combo;

    QRegExpValidator *m_validator = nullptr;

    void limitExtentSize(int);
    bool continueWarning();
    void buildDialog(QList<const StorageBase *> devices);
    QWidget *buildGeneralTab(const QList<const StorageBase *> devices);
    QWidget *buildAdvancedTab();

public:
    explicit VGCreateDialog(QWidget *parent = nullptr);
    explicit VGCreateDialog(StorageBase *const device, QWidget *parent = nullptr);

private slots:

    void validateOK();
    void commit();
    void extentSizeChanged();

};

#endif
