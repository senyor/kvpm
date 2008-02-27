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
#include "removemissing.h"
#include "volgroup.h"

bool remove_missing_pv(VolGroup *VolumeGroup)
{
    QStringList args;
    QString vg_name = VolumeGroup->getName();

    QString message = "Removing missing physical volumes may cause some data in ";
    message.append("volume group <b>" + vg_name + "</b> to become permanently inaccessible!");

    if(KMessageBox::warningYesNo( 0, message) == 3){      // 3 = "yes" button

    args << "/sbin/vgreduce"
	 << "--removemissing"
	 << vg_name;

    ProcessProgress remove(args, "Removing missing volumes...", true);
    return TRUE;
    }
    else
	return FALSE;
}
