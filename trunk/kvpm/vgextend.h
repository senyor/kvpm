/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013, 2014, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef VGEXTEND_H
#define VGEXTEND_H

#include <QComboBox>
#include <QLineEdit>
#include <QList>

#include "kvpmdialog.h"

class PvGroupBox;
class StorageBase;
class VolGroup;

class VGExtendDialog : public KvpmDialog
{
    Q_OBJECT

    PvGroupBox *m_pv_checkbox = nullptr;
    const VolGroup *const m_vg;
    QComboBox *m_copies_combo;
    QLineEdit *m_size_edit;
    QLineEdit *m_align_edit;
    QLineEdit *m_offset_edit;
    
    bool continueWarning();
    void buildDialog(const QList<const StorageBase *> devices);
    QWidget *buildGeneralTab(const QList<const StorageBase *> devices);
    QWidget *buildAdvancedTab();
    
public:
    VGExtendDialog(const VolGroup *const group, QWidget *parent = nullptr);
    VGExtendDialog(const VolGroup *const group, const StorageBase *const device, QWidget *parent = nullptr);

private slots:
    void commit();
    void validateOK();

};

#endif
