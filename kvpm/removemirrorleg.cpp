/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "removemirrorleg.h"

#include <KMessageBox>
#include <KLocale>
#include <QtGui>

#include "logvol.h"
#include "processprogress.h"
#include "volgroup.h"


bool remove_mirror_leg(LogVol *mirrorLeg)
{
    QStringList args;
    LogVol *parent_mirror = mirrorLeg->getParent();

    while( parent_mirror->getParent() != NULL ){
        if( parent_mirror->getParent()->isSnapContainer() )
            break;
        else
            parent_mirror = parent_mirror->getParent();
    }

    QStringList pvs_to_remove  = mirrorLeg->getDevicePathAll();

    QString message = i18n("Remove mirror leg: %1 ?", mirrorLeg->getName());
    
    if(KMessageBox::warningYesNo( 0, message) == 3){      // 3 = "yes" button

	args << "lvconvert"
	     << "--mirrors" 
	     << QString("-1")
	     << parent_mirror->getFullName()
	     << pvs_to_remove;

	ProcessProgress remove(args, i18n("Removing mirror leg..."), true);
	return true;
    }
    else{
	return false;
    }
}
