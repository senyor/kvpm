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

#ifndef VGRENAME_H
#define VGRENAME_H



#include "kvpmdialog.h"

class QLineEdit;
class QRegExpValidator;

class VolGroup;


class VGRenameDialog : public KvpmDialog
{
    Q_OBJECT

    VolGroup  *m_vg = nullptr;
    QString    m_old_name;
    QLineEdit *m_new_name = nullptr;
    QRegExpValidator *m_name_validator = nullptr;

public:
    explicit VGRenameDialog(VolGroup *const group, QWidget *parent = nullptr);

private slots:
    void validateName(QString);
    void commit();

};


#endif
