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
#include "vgextend.h"

bool extend_vg(QString volumeGroupName, QString physicalVolumeName)
{
    QString message;
    QStringList args;
    
    message.append("Do you want to extend volume group: ");
    message.append("<b>" + volumeGroupName + "</b>");
    message.append(" with physical volume: ");
    message.append("<b>" + physicalVolumeName + "</b>?");

    if( KMessageBox::questionYesNo(0, message) == 3 ){     // 3 is the "yes" button

	args << "vgextend"
	     << volumeGroupName
	     << physicalVolumeName;

	ProcessProgress extend(args, "Extending vg...", true);

	return true;
    }
    else{ 
	return false;  // do nothing
    }
}
