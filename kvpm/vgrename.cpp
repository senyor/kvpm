/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <KLocale>
#include <QtGui>

#include "vgrename.h"
#include "volgroup.h"
#include "processprogress.h"

bool rename_vg(VolGroup *volumeGroup)
{
    VGRenameDialog dialog(volumeGroup);
    dialog.exec();
    
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress rename( dialog.arguments(), i18n("Renaming volume group..."), false );
        return true;
    }
    else
        return false;
}

VGRenameDialog::VGRenameDialog(VolGroup *volumeGroup, QWidget *parent) : 
    KDialog(parent)
{

    setWindowTitle( i18n("Rename volume group") );

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    m_old_name = volumeGroup->getName();
    QLabel *old_name_label = new QLabel( i18n("Old volume group name: %1").arg(m_old_name) );
    layout->addWidget(old_name_label);
    
    QLabel *name_label = new QLabel( i18n("New volume group name: ") );
    m_new_name = new KLineEdit();

    QRegExp rx("[0-9a-zA-Z_\\.][-0-9a-zA-Z_\\.]*");
    m_name_validator = new QRegExpValidator( rx, m_new_name );
    m_new_name->setValidator(m_name_validator);
    QHBoxLayout *name_layout = new QHBoxLayout();
    name_layout->addWidget(name_label);
    name_layout->addWidget(m_new_name);
    layout->addLayout(name_layout);
    
    enableButtonOk(false);

    connect(m_new_name, SIGNAL(textChanged(QString)), 
	    this, SLOT(validateName(QString)));
}

QStringList VGRenameDialog::arguments()
{
    QStringList args;
    
    args << "vgrename"
	 << m_old_name
	 << m_new_name->text();

    return args;
}

/* The allowed characters in the name are letters, numbers, periods
   hyphens and underscores. Also the names ".", ".." and names starting
   with a hyphen are disallowed */

void VGRenameDialog::validateName(QString name)
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
