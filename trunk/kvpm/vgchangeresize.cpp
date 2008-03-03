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
	message.append("Volume group: <b>" + vg_name + "</b> is currently resizeable.");
	message.append(" Do you wish to change it to <b>not</b> be resizeable?");
    }
    else{
	message.append("Volume group: <b>" + vg_name + "</b> is currently <b>not</b> resizeable.");
	message.append(" Do you wish to change it to be resizeable?");
    }

    if(KMessageBox::questionYesNo( 0, message) == 3){      // 3 = "yes" button

	args << "vgchange";

	if( VolumeGroup->isResizable() )
	    args << "--resizeable" << "n";
	else
	    args << "--resizeable" << "y";
	
	args << vg_name;
        ProcessProgress resize(args, "Changing vg resize...");
        return TRUE;
    }
    else
        return FALSE;
}
