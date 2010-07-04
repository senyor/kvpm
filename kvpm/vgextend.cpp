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

#include <lvm2app.h>

#include <KMessageBox>
#include <KLocale>
#include <QtGui>

#include "vgextend.h"

bool extend_vg(QString volumeGroupName, QString physicalVolumeName)
{
    QString message;
    QStringList args;
    
    message = i18n("Do you want to extend volume group: <b>%1</b> with "
		   "physical volume: <b>%2</b>").arg(volumeGroupName).arg(physicalVolumeName);

    if( KMessageBox::questionYesNo(0, message) == 3 ){     // 3 is the "yes" button
        int error_number;
        lvm_t  lvm = lvm_init(NULL);

        vg_t vg_dm = lvm_vg_open(lvm, volumeGroupName.toAscii().data(), "w", NULL);
        error_number = lvm_vg_extend(vg_dm, physicalVolumeName.toAscii().data());
        qDebug("Extending VG %d", error_number);
        error_number = lvm_vg_write(vg_dm);
        qDebug("Writing VG %d", error_number);
        lvm_vg_close(vg_dm);
        lvm_quit(lvm);

	return true;
    }
    else{ 
	return false;  // do nothing
    }
}
