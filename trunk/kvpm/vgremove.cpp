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
#include "vgremove.h"
#include "volgroup.h"

bool remove_vg(VolGroup *VolumeGroup)
{
    QStringList args;
    QString message;

    message.append("Are you certain you want to delete volume group: ");
    message.append("<b>" + VolumeGroup->getName() + "</b>");

    if(KMessageBox::questionYesNo( 0, message) == 3){      // 3 = "yes" button
    
	args << "/sbin/vgremove"
	     << VolumeGroup->getName();

	ProcessProgress remove(args, "Removing vg...");
	
	return TRUE;
    }
    else		   
	return FALSE;
}
