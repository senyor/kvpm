/*
 *
 * 
 * Copyright (C) 2009 Benjamin Scott   <benscott@nwlink.com>
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


// Removes all traces of any filesystem on a partition


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
