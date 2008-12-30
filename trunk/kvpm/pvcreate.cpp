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
#include "pvcreate.h"

bool create_pv(QString physicalVolumeName)
{
    QStringList args;
    QString message = i18n("Are you certain you want to create a " 
			   "physical volume on: <b>%1</b>? Any information "
			   "already on that device will be lost." ).arg(physicalVolumeName);

    if(KMessageBox::warningContinueCancel( 0, message) == 5){  // 5 = continue button

	args << "pvcreate" 
	     << "--force"
	     << physicalVolumeName;

	ProcessProgress create(args, i18n("Creating pv..."), false);

	return true;
    }
    else{
	return false;
    }
}

