/*
 *
 *
 * Copyright (C) 2008, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "vgrename.h"

#include <KLocalizedString>

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRegExpValidator>
#include <QStringList>
#include <QVBoxLayout>

#include "logvol.h"
#include "mounttables.h"
#include "processprogress.h"
#include "volgroup.h"



VGRenameDialog::VGRenameDialog(VolGroup *const group, QWidget *parent) :
    KvpmDialog(parent),
    m_vg(group),
    m_old_name(group->getName())
{
    setCaption(i18n("Rename Volume Group"));

    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    QLabel *label = new QLabel(i18n("Rename Volume Group"));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    layout->addSpacing(10);
    label = new QLabel(i18n("Current volume group name: <b>%1</b>", m_old_name));
    layout->addWidget(label);

    QLabel *const name_label = new QLabel(i18n("New volume group name: "));
    m_new_name = new QLineEdit();
    QRegExp rx("[0-9a-zA-Z_\\.][-0-9a-zA-Z_\\.]*");
    m_name_validator = new QRegExpValidator(rx, m_new_name);
    m_new_name->setValidator(m_name_validator);
    QHBoxLayout *const name_layout = new QHBoxLayout();
    name_layout->addWidget(name_label);
    name_layout->addWidget(m_new_name);
    layout->addLayout(name_layout);

    enableButtonOk(false);

    connect(m_new_name, SIGNAL(textChanged(QString)),
            this,       SLOT(validateName(QString)));
}

void VGRenameDialog::commit()
{
    hide();

    QStringList args = QStringList() << "vgrename" 
                                     << m_old_name 
                                     << m_new_name->text();

    const QString old_name = '/' + m_old_name + '/';
    const QString new_name = '/' + m_new_name->text().trimmed() + '/';

    ProcessProgress rename(args);
    if (!rename.exitCode()) {
        for (auto lv : m_vg->getLogicalVolumes()) {
            if (lv->isMounted()) {
                const QString old_path = lv->getMapperPath();
                const QString new_path = lv->getMapperPath().replace(old_path.lastIndexOf(old_name), old_name.size(), new_name);
                MountTables::renameEntries(old_path, new_path);
            }
        }
    }
}

/* The allowed characters in the name are letters, numbers, periods
   hyphens and underscores. Also the names ".", ".." and names starting
   with a hyphen are disallowed */

void VGRenameDialog::validateName(QString name)
{
    int pos = 0;

    if (m_name_validator->validate(name, pos) == QValidator::Acceptable &&
            name != "." &&
            name != "..") {
        enableButtonOk(true);
    } else {
        enableButtonOk(false);
    }
}

