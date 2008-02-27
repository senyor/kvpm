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
#include "processprogress.h"
#include "pvremove.h"


bool remove_pv(QString PhysicalVolumePath)
{
    QStringList args;
    
    QString message = "Remove physical volume on: ";
    message.append("<b>" + PhysicalVolumePath + "</b>?");

    if(KMessageBox::questionYesNo( 0, message) == 3){  // 3 = "yes" button
	args << "/sbin/pvremove" 
	     << "--force"
	     << PhysicalVolumePath;
	ProcessProgress remove(args, "Removing pv...", false);
	return true;
    }
    else
	return false;
}
