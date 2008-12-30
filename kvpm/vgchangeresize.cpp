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

#include "processprogress.h"
#include "vgchangeresize.h"
#include "volgroup.h"


/* This dialog simply toggles the resizable property of
   the volume group and off */

bool change_vg_resize(VolGroup *VolumeGroup)
{
    QStringList args;
    QString vg_name, message;

    vg_name = VolumeGroup->getName();
    
    if( VolumeGroup->isResizable() ){
	message = i18n("Volume group: <b>%1</b> is currently resizeable. "
		       "Do you wish to change it to <b>not</b> be "
		       "resizeable?").arg(vg_name);
    }
    else{
	message =i18n("Volume group: <b>%1</b> is currently <b>not</b> resizeable. "
		      "Do you wish to change it to be resizeable?").arg(vg_name);
    }

    if(KMessageBox::questionYesNo( 0, message) == 3){      // 3 = "yes" button

	args << "vgchange";

	if( VolumeGroup->isResizable() )
	    args << "--resizeable" << "n";
	else
	    args << "--resizeable" << "y";
	
	args << vg_name;
        ProcessProgress resize(args, i18n("Changing vg resize...") );
        return true;
    }
    else
        return false;
}
