/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
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
#include "vgchangeavailable.h"
#include "volgroup.h"


/* This dialog changes the available status of
   the volume group */


bool change_vg_available(VolGroup *volumeGroup)
{
    QStringList args;
    QString vg_name, message;
    int result;
    bool has_mounted = false;
    QList<LogVol *> lv_list = volumeGroup->getLogicalVolumes();
    
    vg_name = volumeGroup->getName();

    for(int x = 0; x < lv_list.size(); x++){
	if( lv_list[x]->isMounted() )
	    has_mounted = true;
    }
    
    if( has_mounted ){
	message = i18n("Groups with mounted logical volumes "
		       "may not be made unavailable");

	KMessageBox::error( 0, message);
	
	return false;
    }
    else{
	
	message = i18n("Make volume group: <b>%1</b> "
		       "available for use?").arg(vg_name);

	result = KMessageBox::questionYesNo( 0, message);
    
	if( result == 3){      // 3 = "yes" button
	    args << "vgchange" 
		 << "--available" << "y"
		 << vg_name;
	}
	else if( result == 4){ // 4 = "no" button
	    args << "vgchange" 
		 << "--available" << "n"
		 << vg_name;
	}
	else{                  // do nothing and return
	    return false;
	}

	ProcessProgress resize(args, i18n("Changing volume group availability..."));

	return true;
    }
    
}
