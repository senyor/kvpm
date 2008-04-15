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
#include "removemirrorleg.h"
#include "volgroup.h"


bool remove_mirror_leg(LogVol *mirrorLeg)
{
    QStringList args;

    VolGroup *vg = mirrorLeg->getVolumeGroup();
    LogVol *mirror = vg->getLogVolByName( mirrorLeg->getOrigin() );
    QStringList pvs_to_remove  = mirrorLeg->getDevicePathAll();

    QString message = "Remove mirror leg: " + mirrorLeg->getName() + " ?";
    
    if(KMessageBox::warningYesNo( 0, message) == 3){      // 3 = "yes" button

	args << "lvconvert"
	     << "--mirrors" 
	     << QString("-1")
	     << mirror->getFullName()
	     << pvs_to_remove;

	ProcessProgress remove(args, "Removing mirror leg...", true);
	return true;
    }
    else{
	return false;
    }
}
