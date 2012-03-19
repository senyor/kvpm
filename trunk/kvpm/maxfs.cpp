/*
 *
 * 
 * Copyright (C) 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "maxfs.h"

#include <KMessageBox>
#include <KLocale>

#include <QtGui>

#include "fsextend.h"
#include "logvol.h"
#include "processprogress.h"
#include "pvextend.h"
#include "storagedevice.h"
#include "storagepartition.h"

bool max_fs(LogVol *logicalVolume)
{
    const QString path = logicalVolume->getMapperPath();
    const QString fs   = logicalVolume->getFilesystem();
    QString full_name  = logicalVolume->getFullName();
    full_name.remove('[').remove(']');
    
    const QString warning = i18n("Extend the filesystem on: %1 to fill the entire volume?", "<b>"+full_name+"</b>");
    const QString error_message = i18n("Extending is only supported for ext2, ext3, ext4, jfs, xfs, ntfs and Reiserfs. "
                                       "The correct executables for file system extension must also be present");

    if( !fs_can_extend(fs) ){
        KMessageBox::error(0, error_message );
        return false;
    }

    if(KMessageBox::warningYesNo(NULL, 
                                 warning, 
                                 QString(), 
                                 KStandardGuiItem::yes(), 
                                 KStandardGuiItem::no(), 
                                 QString(), 
                                 KMessageBox::Dangerous) == KMessageBox::Yes){
        return fs_extend(path, fs, logicalVolume->getMountPoints(), true); 
    }
    else{
        return false;
    }
}

bool max_fs(StoragePartition *partition)
{
    const QString path = partition->getName();
    const QString fs = partition->getFilesystem();
    const QString error_message = i18n("Filesystem extending is only supported for ext2, ext3, ext4, jfs, xfs, ntfs and Reiserfs. "
                                       "Physical volumes can also be extended. "
                                       "The correct executables for file system extension must be present");

    QString message;

    if( partition->isPhysicalVolume() )
        message = i18n("Extend the physical volume on: %1 to fill the entire partition?", "<b>"+path+"</b>");
    else
        message = i18n("Extend the filesystem on: %1 to fill the entire partition?", "<b>"+path+"</b>");


    if( ! ( fs_can_extend(fs) || partition->isPhysicalVolume() ) ){
        KMessageBox::error(0, error_message );
        return false;
    }

    if(KMessageBox::warningYesNo(NULL, 
                                 message, 
                                 QString(), 
                                 KStandardGuiItem::yes(), 
                                 KStandardGuiItem::no(), 
                                 QString(), 
                                 KMessageBox::Dangerous) == KMessageBox::Yes){
        if( partition->isPhysicalVolume() ){
            return pv_extend(path);
        } 
        else{
            return fs_extend(path, fs, partition->getMountPoints(), false); 
        }
    }
    
    return false;
}

bool max_fs(StorageDevice *device)
{
    const QString path = device->getName();
    const QString warning = i18n("Extend the physical volume on: %1 to fill the entire partition?", "<b>"+path+"</b>");

    if( ! device->isPhysicalVolume() )
        return false;

    if(KMessageBox::warningYesNo(NULL, 
                                 warning, 
                                 QString(), 
                                 KStandardGuiItem::yes(), 
                                 KStandardGuiItem::no(), 
                                 QString(), 
                                 KMessageBox::Dangerous) == KMessageBox::Yes){
        return pv_extend(path); 
    }

    return false;
}
