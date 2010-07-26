/*
 *
 * 
 * Copyright (C) 2008 2010 Benjamin Scott   <benscott@nwlink.com>
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

#include "lvremove.h"
#include "logvol.h"
#include "processprogress.h"


bool remove_lv(LogVol *logicalVolume)
{
    QString full_name = logicalVolume->getFullName();
    QStringList args;
    
    QString message = i18n("Are you certain you want to delete the logical volume named: %1 "
			   "Any data on it will be lost.").arg("<b>" + full_name + "</b>");

    if( KMessageBox::warningYesNo( 0, message) == 3){  // 3 = yes button
        
        if( logicalVolume->isActive() && !logicalVolume->isSnap() && !logicalVolume->isOrigin() ){
            args << "lvchange" 
                 << "-an" 
                 << full_name;

	ProcessProgress deactive( args, i18n("Deactivating volume..."), false);
        }
        
        args.clear();
	args << "lvremove" 
	     << "--force" 
	     << full_name;

	ProcessProgress remove( args, i18n("Removing volume..."), false);

	return true;
    }
    else
	return false;
}

