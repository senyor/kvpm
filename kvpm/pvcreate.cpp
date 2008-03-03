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
#include "pvcreate.h"

bool create_pv(QString PhysicalVolumePath)
{
    QStringList args;
    QString message;
    
    message.append("Are you certain you want to create a physical volume on: ");
    message.append("<b>" + PhysicalVolumePath + "</b>?");
    message.append(" Any information already on that device will be lost.");

    if(KMessageBox::warningContinueCancel( 0, message) == 5){  // 5 = continue button

	args << "pvcreate" 
	     << "--force"
	     << PhysicalVolumePath;

	ProcessProgress create(args, "Creating pv...", false);
	return TRUE;
    }
    else
	return FALSE;
}
