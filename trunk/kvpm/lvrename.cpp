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

#include "lvrename.h"

#include <KLineEdit>
#include <KLocale>

#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QRegExpValidator>
#include <QVBoxLayout>


#include "logvol.h"
#include "mounttables.h"
#include "volgroup.h"
#include "processprogress.h"


LVRenameDialog::LVRenameDialog(LogVol *const volume, QWidget *parent) : 
    KvpmDialog(parent),
    m_lv(volume)
{
    setCaption(i18n("Rename Logical Volume"));

    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    m_vg_name  = m_lv->getVg()->getName();
    m_old_name = m_lv->getName();

    QLabel *label;
    if (m_lv->isThinPool())
        label = new QLabel(i18n("Rename Thin Pool"));
    else
        label = new QLabel(i18n("Rename Logical Volume"));

    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    layout->addSpacing(10);

    label = new QLabel(i18n("Current name: %1", m_old_name));
    layout->addWidget(label);

    QRegExp rx("[0-9a-zA-Z_\\.][-0-9a-zA-Z_\\.]*");
    m_new_name = new KLineEdit();
    m_name_validator = new QRegExpValidator(rx, m_new_name);
    m_new_name->setValidator(m_name_validator);
    QHBoxLayout *const name_layout = new QHBoxLayout();
    label = new QLabel(i18n("New name: "));

    name_layout->addWidget(label);
    name_layout->addWidget(m_new_name);
    layout->addLayout(name_layout);

    enableButtonOk(false);

    connect(m_new_name, SIGNAL(textChanged(QString)),
            this, SLOT(validateName(QString)));
}

/* The allowed characters in the name are letters, numbers, periods
   hyphens and underscores. Also the names ".", ".." and names starting
   with a hyphen are disallowed */

void LVRenameDialog::validateName(QString name)
{
    int pos = 0;

    if (m_name_validator->validate(name, pos) == QValidator::Acceptable &&
            name != "." &&
            name != "..") {
        enableButtonOk(true);
    } else
        enableButtonOk(false);
}

QString LVRenameDialog::getNewMapperPath()
{
    QString path = m_lv->getMapperPath();

    path.truncate(path.lastIndexOf('/') + 1);

    return QString(path + m_new_name->text());
}

void LVRenameDialog::commit()
{
    QStringList args;

    args << "lvrename"
         << m_vg_name
         << m_old_name
         << m_new_name->text();

    ProcessProgress rename(args);

    if (!rename.exitCode() && m_lv->isMounted())
        MountTables::renameEntries(m_lv->getMapperPath(), getNewMapperPath());
}
