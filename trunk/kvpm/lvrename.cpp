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

#include "lvrename.h"

#include <KLocale>
#include <QtGui>

#include "logvol.h"
#include "mounttables.h"
#include "volgroup.h" 
#include "processprogress.h"


LVRenameDialog::LVRenameDialog(LogVol *const volume, QWidget *parent) 
  : KDialog(parent), 
    m_lv(volume)
{
    setWindowTitle( i18n("Rename Logical Volume") );

    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    m_vg_name  = m_lv->getVg()->getName();
    m_old_name = m_lv->getName();

    QLabel *label = new QLabel( i18n("<b>Rename Logical Volume</b>") );
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    layout->addSpacing(10);

    label = new QLabel( i18n("Current volume name: %1", m_old_name) );
    layout->addWidget(label);

    QRegExp rx("[0-9a-zA-Z_\\.][-0-9a-zA-Z_\\.]*");
    m_new_name = new KLineEdit();
    m_name_validator = new QRegExpValidator( rx, m_new_name );
    m_new_name->setValidator(m_name_validator);
    QHBoxLayout *const name_layout = new QHBoxLayout();
    label = new QLabel( i18n("New volume name: ") );

    name_layout->addWidget(label);
    name_layout->addWidget(m_new_name);
    layout->addLayout(name_layout);
    
    enableButtonOk(false);

    connect(this, SIGNAL(okClicked()), 
            this, SLOT(commitChanges()));

    connect(m_new_name, SIGNAL(textChanged(QString)), 
	    this, SLOT(validateName(QString)));
}

/* The allowed characters in the name are letters, numbers, periods
   hyphens and underscores. Also the names ".", ".." and names starting
   with a hyphen are disallowed */

void LVRenameDialog::validateName(QString name)
{
    int pos = 0;

    if( m_name_validator->validate(name, pos) == QValidator::Acceptable &&
	name != "." && 
	name != ".." )
    {
	enableButtonOk(true);
    }
    else
	enableButtonOk(false);
}

QString LVRenameDialog::getNewMapperPath()
{
    QString path = m_lv->getMapperPath();

    path.truncate( path.lastIndexOf('/') + 1 );

    return QString( path + m_new_name->text() );
}

void LVRenameDialog::commitChanges()
{
    QStringList args;
    
    args << "lvrename"
	 << m_vg_name
	 << m_old_name
	 << m_new_name->text();

    ProcessProgress rename(args);

    if( !rename.exitCode() && m_lv->isMounted() )
        MountTables::renameEntries( m_lv->getMapperPath(), getNewMapperPath() );
}
