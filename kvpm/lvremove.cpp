/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "lvremove.h"

#include <KMessageBox>
#include <KLocale>
#include <QtGui>

#include "logvol.h"
#include "processprogress.h"


bool remove_lv(LogVol *logicalVolume)
{
    QString full_name = logicalVolume->getFullName();
    full_name.remove('[').remove(']');
    QStringList args;
    
    QString message = i18n("Are you certain you want to delete the logical volume named: %1 "
			   "Any data on it will be lost.", "<b>" + full_name + "</b>");

    QString message2 = i18n("This volume has snapshots that must be deleted first");

    if( logicalVolume->isOrigin() ){
        KMessageBox::error( 0, message2);
        return false; 
    }

    if(KMessageBox::warningYesNo( 0, message) == 3){  // 3 = yes button
        
        if( logicalVolume->isActive() && !logicalVolume->isSnap() ){
            args << "lvchange" << "-an" << full_name;

            ProcessProgress deactivate( args, i18n("Deactivating volume..."), true);
        }

        args.clear();
        args << "lvremove" << "--force" << full_name;
        
        ProcessProgress remove( args, i18n("Removing volume..."), true);
        
        return true;
    }
    else
        return false;
}

