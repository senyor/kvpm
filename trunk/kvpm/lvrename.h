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

bool rename_lv(LogVol *logicalVolume);

class LVRenameDialog : public KDialog
{
Q_OBJECT

    QString m_old_name;
    QString m_vg_name;
    KLineEdit *m_new_name;
    QRegExpValidator *m_name_validator;
    LogVol *m_lv;

public:
    explicit LVRenameDialog(LogVol *logicalVolume, QWidget *parent = 0);
    QStringList arguments();

private slots:
    void validateName(QString);
    void renameMountEntries();

};


#endif
