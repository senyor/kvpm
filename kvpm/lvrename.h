/*
 *
 *
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
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

#include <KDialog>
#include <KLineEdit>
#include <QStringList>
#include <QRegExpValidator>

class LogVol;


class LVRenameDialog : public KDialog
{
    Q_OBJECT

    LogVol  *m_lv;
    QString  m_old_name;
    QString  m_vg_name;
    KLineEdit *m_new_name;
    QRegExpValidator *m_name_validator;

    QString getNewMapperPath();

public:
    explicit LVRenameDialog(LogVol *const volume, QWidget *parent = 0);

private slots:
    void validateName(QString name);
    void commitChanges();

};


#endif
