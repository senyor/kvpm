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

#include "logvol.h"
#include "processprogress.h"
#include "vgexport.h"
#include "volgroup.h"


bool export_vg(VolGroup *volumeGroup)
{

    QStringList args;

    QString message = i18n("Export volume group: %1 ?").arg(volumeGroup->getName());
    
    if(KMessageBox::questionYesNo( 0, message) == 3){      // 3 = "yes" button

	args << "vgexport"
	     << volumeGroup->getName();
	
	ProcessProgress remove(args, i18n("Exporting volume group..."), true);
	return true;
    }
    else
	return false;
}
