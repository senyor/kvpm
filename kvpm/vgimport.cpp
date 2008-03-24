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

#include "logvol.h"
#include "processprogress.h"
#include "vgimport.h"
#include "volgroup.h"


bool import_vg(VolGroup *volumeGroup)
{

    QStringList args;

    QString message = "Import volume group: " + volumeGroup->getName() + " ?";
    
    if(KMessageBox::questionYesNo( 0, message) == 3){      // 3 = "yes" button

	args << "vgimport"
	     << volumeGroup->getName();
	
	ProcessProgress remove(args, "Importing volume group...", true);
	return true;
    }
    else
	return false;
}
