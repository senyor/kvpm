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
    QStringList args;
    
    QString message = "Are you certain you want to delete the logical volume named: ";
    message.append("<b>" + fullName + "</b>");
    message.append(" Any data on it will be lost.");

    if( KMessageBox::warningYesNo( 0, message) == 3){  // 3 = yes button
    
	args << "lvremove" 
	     << "--force" 
	     << fullName;

	ProcessProgress remove( args, "Removing volume...", false);

	return true;
    }
    else
	return false;
}

