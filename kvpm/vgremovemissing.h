/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef VGREMOVEMISSING_H
#define VGREMOVEMISSING_H


class QRadioButton;

class VolGroup;

#include "kvpmdialog.h"


class VGRemoveMissingDialog : public KvpmDialog
{
    Q_OBJECT

    VolGroup *m_vg = nullptr;

    QRadioButton *m_empty_button,
                 *m_all_button;

public:
    explicit VGRemoveMissingDialog(VolGroup *const group, QWidget *parent = nullptr);

private slots:
    void commit();

};

#endif
