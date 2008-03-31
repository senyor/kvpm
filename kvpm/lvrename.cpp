/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <QtGui>

#include "lvrename.h"
#include "logvol.h"
#include "mountentry.h"
#include "volgroup.h" 
#include "processprogress.h"

bool rename_lv(LogVol *logicalVolume)
{
    LVRenameDialog dialog(logicalVolume);
    dialog.exec();
    
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress rename( dialog.arguments(), "Renaming logical volume...", false );

	return true;
    }
    else{
        return false;
    }
}

LVRenameDialog::LVRenameDialog(LogVol *logicalVolume, QWidget *parent) : 
    KDialog(parent)
{

    setWindowTitle(tr("Rename logical volume"));

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    m_vg_name  = logicalVolume->getVolumeGroupName();
    m_old_name = logicalVolume->getName();

    QLabel *old_name_label = new QLabel("Old logical volume name: " + m_old_name);
    layout->addWidget(old_name_label);

    QLabel *name_label = new QLabel("New logical volume name: ");
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

    connect(this, SIGNAL(okClicked()), 
	    this, SLOT(renameMountEntries()));
}

QStringList LVRenameDialog::arguments()
{
    QStringList args;
    
    args << "lvrename"
	 << m_vg_name
	 << m_old_name
	 << m_new_name->text();

    return args;
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

void LVRenameDialog::renameMountEntries()
{
    rename_mount_entries("/dev/mapper/" + m_vg_name + "-" + m_old_name, 
			 "/dev/mapper/" + m_vg_name + "-" + m_new_name->text());
}

