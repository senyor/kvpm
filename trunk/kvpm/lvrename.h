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

#ifndef LVRENAME_H
#define LVRENAME_H


#include "kvpmdialog.h"
#include "typedefs.h"


class KLineEdit;

class QRegExpValidator;
class QString;

class LogVol;


class LVRenameDialog : public KvpmDialog
{
    Q_OBJECT

    LvPtr m_lv;
    QString  m_old_name;
    QString  m_vg_name;
    KLineEdit *m_new_name;
    QRegExpValidator *m_name_validator;

    QString getNewMapperPath();

public:
    explicit LVRenameDialog(LvPtr volume, QWidget *parent = nullptr);

private slots:
    void validateName(QString name);
    void commit();

};


#endif
