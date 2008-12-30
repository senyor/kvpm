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


#include <KLocale>
#include <KMessageBox>
#include <QtGui>

#include "processprogress.h"
#include "pvremove.h"


bool remove_pv(QString PhysicalVolumePath)
{
    QStringList args;
    
    QString message = i18n("Remove physical volume on: <b>%1</b>?").arg(PhysicalVolumePath);

    if(KMessageBox::questionYesNo( 0, message) == 3){  // 3 = "yes" button
	args << "pvremove" 
	     << "--force"
	     << PhysicalVolumePath;
	ProcessProgress remove(args, i18n("Removing physical volume..."), false);
	return true;
    }
    else
	return false;
}
