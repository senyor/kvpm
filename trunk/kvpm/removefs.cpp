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


#include <parted/parted.h>

#include <KMessageBox>
#include <KLocale>

#include <QtGui>

#include "removefs.h"
#include "storagepartition.h"
#include "logvol.h"

// Removes all traces of any filesystem on a partition or volume


bool remove_fs(StoragePartition *partition)
{

    PedGeometry geometry = partition->getPedPartition()->geom;

    QString message = i18n("Are you sure you want delete this filesystem? "
                           "Any data on it will be lost!");

    if(KMessageBox::warningContinueCancel(0, message) == KMessageBox::Continue){

        if( ped_file_system_clobber( &geometry ) )
            return true;
        else
            return false;
    }
    else
        return false;

}

bool remove_fs(LogVol *logicalVolume)
{

    QString message = i18n("Are you sure you want delete this filesystem? "
                           "Any data on it will be lost!");

    QByteArray zero_array(128 * 1024, '\0');
    QString path = logicalVolume->getMapperPath();
    QFile *device;

    if(KMessageBox::warningContinueCancel(0, message) == KMessageBox::Continue){

        device = new QFile(path);

        if( device->open(QIODevice::ReadWrite) ){
            device->write(zero_array);
            device->flush();
            device->close();

            return(true);
        }
    }
        
    return(false);
}
