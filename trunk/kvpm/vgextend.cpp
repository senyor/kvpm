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
#include "processprogress.h"
#include "vgextend.h"

bool extend_vg(QString VolumeGroupName, QString PhysicalVolumePath)
{
    QString message;
    QStringList args;
    
    message.append("Do you want to extend volume Group: ");
    message.append("<b>" + VolumeGroupName + "</b>");
    message.append(" with Physical Volume: ");
    message.append("<b>" + PhysicalVolumePath + "</b>?");

    if( KMessageBox::questionYesNo(0, message) == 3 ){     // 3 is the "yes" button

	args << "/sbin/vgextend"
	     << VolumeGroupName
	     << PhysicalVolumePath;

	ProcessProgress extend(args);
	return TRUE;
    }
    else
	return FALSE;
}
