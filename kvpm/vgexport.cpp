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


#include "vgexport.h"

#include <KLocale>
#include <KMessageBox>

#include <QtGui>

#include "logvol.h"
#include "processprogress.h"
#include "volgroup.h"


bool export_vg(VolGroup *const volumeGroup)
{
    QStringList args;
    const QString message = i18n("Export volume group: <b>%1</b>?", volumeGroup->getName() );
    
    if(KMessageBox::questionYesNo(0, message) == KMessageBox::Yes){

	args << "vgexport"
	     << volumeGroup->getName();
	
	ProcessProgress remove(args);
	return true;
    }
    else
	return false;
}
