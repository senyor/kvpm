/*
 *
 * 
 * Copyright (C) 2011 Benjamin Scott   <benscott@nwlink.com>
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
#include <KLocale>
#include <QtGui>

#include "maxfs.h"
#include "fsextend.h"
#include "logvol.h"
#include "processprogress.h"
#include "pvextend.h"
#include "storagedevice.h"
#include "storagepartition.h"

bool max_fs(LogVol *logicalVolume)
{
    QString path = logicalVolume->getMapperPath();
    QString full_name = logicalVolume->getFullName();
    full_name.remove('[').remove(']');
    QStringList args;
    QString fs = logicalVolume->getFilesystem();
    
    QString message = i18n("Extend the filesystem on: %1 to fill the entire volume?").arg("<b>"+full_name+"</b>");
    QString error_message = i18n("Extending is only supported for ext2/3/4, jfs, xfs and Reiserfs. ");

    if( ! ( fs == "ext2" || fs == "ext3" || fs == "ext4" || fs == "reiserfs" || fs == "xfs"  || fs == "jfs" ) ){
        KMessageBox::error(0, error_message );
        return false;
    }

    if(KMessageBox::warningYesNo( 0, message) == 3){  // 3 = yes button
        return fs_extend( path, fs, true); 
    }
    else
        return false;
}

bool max_fs(StoragePartition *partition)
{

    QString path = partition->getName();
    QString fs = partition->getFilesystem();
    QString message, error_message;

    if( partition->isPhysicalVolume() )
        message = i18n("Extend the physical volume on: %1 to fill the entire partition?", "<b>"+path+"</b>");
    else
        message = i18n("Extend the filesystem on: %1 to fill the entire partition?", "<b>"+path+"</b>");

    error_message = i18n("Extending is only supported for ext2/3/4, jfs, xfs, Reiserfs and physical volumes. ");

    if( ! ( fs == "ext2" || fs == "ext3" || fs == "ext4" || fs == "reiserfs" ||
            fs == "xfs"  || fs == "jfs"  || partition->isPhysicalVolume() ) ){

        KMessageBox::error(0, error_message );
        return false;
    }

    if(KMessageBox::warningYesNo( 0, message) == 3){  // 3 = yes button
        if( partition->isPhysicalVolume() )
            return pv_extend(path); 
        else
            return fs_extend(path, fs, true); 
    }

    return false;
}

bool max_fs(StorageDevice *device)
{

    QString path = device->getName();

    QString message = i18n("Extend the physical volume on: %1 to fill the entire partition?", "<b>"+path+"</b>");

    if( ! device->isPhysicalVolume() )
        return false;

    if(KMessageBox::warningYesNo( 0, message) == 3){  // 3 = yes button
        return pv_extend(path); 
    }

    return false;
}
