/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include <parted/parted.h>

#include <KLocale>
#include <KMessageBox>

#include <QtGui>

#include "partremove.h"
#include "storagepartition.h"


bool remove_partition(StoragePartition *const partition)
{
    QString message;
    PedPartition *const ped_partition = partition->getPedPartition();

    if( partition->getType() != "extended" ){
        message = i18n("Remove partition: <b>%1?</b> Any data on that partition will be lost.", partition->getName() );

        if(KMessageBox::warningYesNo(0, message) == KMessageBox::Yes){

            if( ped_disk_delete_partition( ped_partition->disk, ped_partition ) )
                ped_disk_commit( ped_partition->disk );
            
            return true;
        }
    }
    else{
        message = i18n("Remove partition: <b>%1?</b>", partition->getName() );

        if(KMessageBox::questionYesNo(0, message) == KMessageBox::Yes){
            
            if( ped_disk_delete_partition( ped_partition->disk, ped_partition ) )
                ped_disk_commit( ped_partition->disk );
            
            return true;
        }
    }

    return false;
}
