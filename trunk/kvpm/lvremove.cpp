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

bool remove_lv(LogVol *LogicalVolume)
{
    return (remove_lv( LogicalVolume->getFullName() ));
}

bool remove_lv(QString FullName)
{
    LVRemoveDialog dialog(FullName);
    if(dialog.result() == QDialog::Accepted){
	ProcessProgress remove_lv(dialog.arguments(), "Removing volume...", FALSE);
	return TRUE;
    }
    else
	return FALSE;
}

LVRemoveDialog::LVRemoveDialog(QString FullName, QWidget *parent):QObject(parent)
{
    lv_full_name = FullName;
    
    QString message = "Are you certain you want to delete the logical volume named: ";
    message.append("<b>" + lv_full_name + "</b>");
    message.append(" Any data on it will be lost.");
    
    return_code =  KMessageBox::warningYesNo( 0, message);
}

QStringList LVRemoveDialog::arguments()
{
    QStringList args;
    
    args << "/sbin/lvremove" 
	 << "--force" 
	 << lv_full_name;

    return args;
}

int LVRemoveDialog::result()
{
    if(return_code == 3)  // 3 = yes button
        return 1;         // QDialog::Accepted
    else
        return 0;         // QDialog::Rejected
}
