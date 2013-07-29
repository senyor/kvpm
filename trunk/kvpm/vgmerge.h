/*
 *
 *
 * Copyright (C) 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef VGMERGE_H
#define VGMERGE_H

#include <stdint.h>

#include <KDialog>

#include <QList>


#include "kvpmdialog.h"

class KComboBox;

class QCheckBox;
class QStackedWidget;

class VolGroup;


class VGMergeDialog : public KvpmDialog
{
    Q_OBJECT

    VolGroup  *m_vg;
    KComboBox *m_target_combo;
    QCheckBox *m_autobackup;
    QStackedWidget *m_error_stack;
    QList<uint64_t> m_extent_size;

    void checkSanity();

public:
    explicit VGMergeDialog(VolGroup *const volumeGroup, QWidget *parent = nullptr);

private slots:
    void commit();
    void compareExtentSize();

};

#endif
