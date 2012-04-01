/*
 *
 *
 * Copyright (C) 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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

#include <KDialog>
#include <QList>

class QCheckBox;
class QStackedWidget;

class KComboBox;

class VolGroup;


class VGMergeDialog : public KDialog
{
    Q_OBJECT

    VolGroup  *m_vg;
    KComboBox *m_target_combo;
    QCheckBox *m_autobackup;
    QStackedWidget *m_error_stack;
    QList<long> m_extent_size;

public:
    explicit VGMergeDialog(VolGroup *const volumeGroup, QWidget *parent = 0);
    bool bailout();

private slots:
    void commitChanges();
    void compareExtentSize();

};

#endif
