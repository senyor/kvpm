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
#include "removemissing.h"
#include "volgroup.h"

bool remove_missing_pv(VolGroup *volumeGroup)
{
    QStringList args;
    QString vg_name = volumeGroup->getName();

    QString message = i18n("Removing missing physical volumes may cause some "
			   "data in volume group <b>%1</b> to become permanently "
			   "inaccessible!").arg(vg_name);

    if(KMessageBox::warningYesNo( 0, message) == 3){      // 3 = "yes" button

	args << "vgreduce"
	     << "--removemissing"
	     << vg_name;

	ProcessProgress remove(args, i18n("Removing missing volumes..."), true);

	return true;
    }
    else{
	return false;
    }
}
