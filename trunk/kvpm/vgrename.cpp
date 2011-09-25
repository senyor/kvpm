/*
 *
 * 
 * Copyright (C) 2008, 2011 Benjamin Scott   <benscott@nwlink.com>
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

#include <KLocale>
#include <QtGui>

#include "logvol.h"
#include "mountentry.h"
#include "processprogress.h"
#include "volgroup.h"


bool rename_vg(VolGroup *volumeGroup)
{
    QList<LogVol *> lvs;
    QString new_path, old_path;
    QString new_name, old_name;

    VGRenameDialog dialog(volumeGroup);
    dialog.exec();

    if(dialog.result() == QDialog::Accepted){
        ProcessProgress rename( dialog.arguments(), i18n("Renaming volume group..."), false );

        if( ! rename.exitCode() ){
            lvs = volumeGroup->getLogicalVolumes();
            old_name = '/' + dialog.getOldName() + '/';
            new_name = '/' + dialog.getNewName() + '/';

            for(int x = lvs.size() - 1; x >= 0; x--){
                if( lvs[x]->isMounted() ){
                    old_path = lvs[x]->getMapperPath();
                    new_path = lvs[x]->getMapperPath().replace(old_path.lastIndexOf(old_name), old_name.size(), new_name);
                    rename_mount_entries(old_path, new_path);
                }
            }
        }
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
    QLabel *old_name_label = new QLabel( i18n("Old volume group name: %1", m_old_name) );
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

QString VGRenameDialog::getNewName()
{
    return m_new_name->text().trimmed();
}

QString VGRenameDialog::getOldName()
{
    return m_old_name;
}
