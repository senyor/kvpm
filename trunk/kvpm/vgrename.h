/*
 *
 * 
 * Copyright (C) 2008, 2011 Benjamin Scott   <benscott@nwlink.com>
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

#include <KDialog>
#include <KLineEdit>

#include <QStringList>
#include <QRegExpValidator>

class VolGroup;

bool rename_vg(VolGroup *volumeGroup);

class VGRenameDialog : public KDialog
{
Q_OBJECT

    QString m_old_name;
    KLineEdit *m_new_name;
    QRegExpValidator *m_name_validator;

public:
    explicit VGRenameDialog(VolGroup *volumeGroup, QWidget *parent = 0);
    QStringList arguments();
    QString getNewName();
    QString getOldName();

private slots:
    void validateName(QString);

};


#endif
