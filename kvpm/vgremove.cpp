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
#include <KLocale>
#include <QtGui>

#include "processprogress.h"
#include "vgremove.h"
#include "volgroup.h"

bool remove_vg(VolGroup *volumeGroup)
{
    QStringList args;
    QString message;

    message = i18n("Are you certain you want to "
		   "delete volume group: <b>%1</b>").arg(volumeGroup->getName());


    if(KMessageBox::questionYesNo( 0, message) == 3){      // 3 = "yes" button
    
	args << "vgremove"
	     << volumeGroup->getName();

	ProcessProgress remove(args, i18n("Removing volume group...") );
	
	return true;
    }
    else{		   
	return false;
    }
}
