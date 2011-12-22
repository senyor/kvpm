/*
 *
 * 
 * Copyright (C) 2008, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "vgreduceone.h"

#include <KMessageBox>
#include <KLocale>
#include <QtGui>

#include "processprogress.h"


bool reduce_vg_one(QString volumeGroupName, QString physicalVolumeName)
{
    QStringList args;
    const QString message = i18n( "Remove physical volume: <b>%1</b> from volume " 
                                  "group: <b>%2</b>", physicalVolumeName, volumeGroupName );

    if(KMessageBox::questionYesNo(0, message) == KMessageBox::Yes){

	args << "vgreduce"
	     << volumeGroupName
	     << physicalVolumeName;

	ProcessProgress remove(args);

	return true;
    }
    else{
	return false;
    }
}
