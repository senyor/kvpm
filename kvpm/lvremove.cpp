/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Klvm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <KMessageBox>
#include <QtGui>
#include "lvremove.h"
#include "logvol.h"
#include "processprogress.h"

bool remove_lv(LogVol *logicalVolume)
{
    return (remove_lv( logicalVolume->getFullName() ));
}

bool remove_lv(QString fullName)
{
    LVRemoveDialog dialog(fullName);
    if(dialog.result() == QDialog::Accepted){
	ProcessProgress remove_lv(dialog.arguments(), "Removing volume...", false);
	return true;
    }
    else
	return false;
}

LVRemoveDialog::LVRemoveDialog(QString fullName, QWidget *parent):QObject(parent)
{
    m_lv_full_name = fullName;
    
    QString message = "Are you certain you want to delete the logical volume named: ";
    message.append("<b>" + m_lv_full_name + "</b>");
    message.append(" Any data on it will be lost.");
    
    m_return_code =  KMessageBox::warningYesNo( 0, message);
}

QStringList LVRemoveDialog::arguments()
{
    QStringList args;
    
    args << "/sbin/lvremove" 
	 << "--force" 
	 << m_lv_full_name;

    return args;
}

int LVRemoveDialog::result()
{
    if(m_return_code == 3)  // 3 = yes button
        return 1;           // QDialog::Accepted
    else
        return 0;           // QDialog::Rejected
}
