/*
 *
 * 
 * Copyright (C) 2009, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include "fsck.h"

#include <KLocale>
#include <KMessageBox>

#include <QtGui>

#include "logvol.h"
#include "processprogress.h"
#include "storagepartition.h"

bool fsck(const QString path){

    QStringList arguments, output;

    arguments << "fsck" 
              << "-fp" 
              << path; 

    ProcessProgress fsck_fs(arguments);
    output = fsck_fs.programOutput();

    if ( fsck_fs.exitCode() > 1 )   // 0 means no errors, 1 means minor errors fixed
        return false;
    else
        return true;
}

bool manual_fsck(LogVol *const logicalVolume){

    const QString path = logicalVolume->getMapperPath();
    const QString message = i18n("Run <b>'fsck -fp'</b> to check the filesystem on volume <b>%1?</b>", path);

    if(KMessageBox::questionYesNo(0, message) == KMessageBox::Yes)
        return fsck(path);
    else
        return false;
}

bool manual_fsck(StoragePartition *const partition){

    const QString path = partition->getName();
    const QString message = i18n("Run <b>'fsck -fp'</b> to check the filesystem on partition <b>%1?</b>", path);

    if(KMessageBox::questionYesNo(0, message) == KMessageBox::Yes)
        return fsck(path);
    else
        return false;
}
