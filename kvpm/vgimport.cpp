/*
 *
 * 
 * Copyright (C) 2008, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "vgimport.h"

#include <KMessageBox>
#include <KLocale>

#include <QtGui>

#include "logvol.h"
#include "processprogress.h"
#include "volgroup.h"


bool import_vg(VolGroup *volumeGroup)
{
    QStringList args;
    const QString message = i18n("Import volume group: %1 ?", volumeGroup->getName());
    
    if(KMessageBox::questionYesNo( 0, message) == 3){      // 3 = "yes" button

	args << "vgimport"
	     << volumeGroup->getName();
	
	ProcessProgress remove(args);
	return true;
    }
    else{
	return false;
    }
}
