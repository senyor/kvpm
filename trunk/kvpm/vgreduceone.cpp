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
#include "vgreduceone.h"
#include "processprogress.h"


bool reduce_vg_one(QString VolumeGroupName, QString PhysicalVolumePath)
{

    QStringList args;
 
    QString message = "Remove physical volume: <b>" + PhysicalVolumePath + "</b>";
    message.append(" from volume group: <b>" + VolumeGroupName + "</b>");
    

    int return_code =  KMessageBox::questionYesNo( 0, message);

    if(return_code == 3){  // 3 = yes button
	args << "vgreduce"
	     << VolumeGroupName
	     << PhysicalVolumePath;
	ProcessProgress remove( args, "Reducing vg...", true );
	return TRUE;
    }
    else
	return FALSE;
}
