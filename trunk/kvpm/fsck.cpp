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

#include <KLocale>
#include <KMessageBox>

#include <QtGui>

#include "logvol.h"
#include "processprogress.h"
#include "storagepartition.h"

bool fsck(QString path){

    QStringList arguments, output;

    arguments << "fsck" 
              << "-fp" 
              << path; 

    ProcessProgress fsck_fs(arguments, i18n("Checking filesystem..."), true );
    output = fsck_fs.programOutput();

    if ( fsck_fs.exitCode() > 1 )   // 0 means no errors 1 means minor errors fixed
        return false;
    else
        return true;
}

bool manual_fsck(LogVol *logicalVolume){

    const QString path = logicalVolume->getMapperPath();
    const QString message = i18n("<b>Run 'fsck -fp' on %1?</b>", path);

    if(KMessageBox::warningYesNo( 0, message) == 3)  // 3 = yes button
        return fsck(path);
    else
        return false;
}

bool manual_fsck(StoragePartition *partition){

    const QString path = partition->getName();
    const QString message = i18n("<b>Run 'fsck -fp' on %1?</b>", path);

    if(KMessageBox::warningYesNo( 0, message) == 3)  // 3 = yes button
        return fsck(path);
    else
        return false;
}