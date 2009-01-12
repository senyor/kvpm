/*
 *
 * 
 * Copyright (C) 2008, 2009 Benjamin Scott   <benscott@nwlink.com>
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

bool remove_partition( StoragePartition *partition )
{
    QStringList args;
    PedPartition *ped_partition;

    QString message = i18n("Remove partition: <b>%1</b>? "
			   "Any data on that partition will be lost!",
			   partition->getPartitionPath() );

    ped_partition = partition->getPedPartition();

    if(KMessageBox::warningContinueCancel( 0, message) == 5){  // 5 = "continue" button
        
        if(ped_disk_delete_partition( ped_partition->disk, ped_partition ) ){
	    ped_disk_commit (ped_partition->disk);
	    return true;
	}
	else{
	    KMessageBox::error( 0, "partition removal failed");
	    return false;
	}
    }
    else
        return false;
}
