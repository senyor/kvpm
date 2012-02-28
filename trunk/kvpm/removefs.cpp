/*
 *
 * 
 * Copyright (C) 2009, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "removefs.h"

#include <KMessageBox>
#include <KLocale>

#include <QtGui>

#include "storagepartition.h"
#include "logvol.h"



// Removes all traces of any filesystem on a partition or volume

bool remove_fs(const QString name)
{
    const QString warning_message = i18n("Are you sure you want delete the filesystem on <b>%1</b>? "
                                         "Any data on it will be lost.", name);

    const QString error_message = i18n("Error writing to device %1", name);

    if(KMessageBox::warningYesNo(0, warning_message) == KMessageBox::Yes){

        QByteArray zero_array(128 * 1024, '\0');
        QFile *const device = new QFile(name);
        bool error = false;

        if( device->open(QIODevice::ReadWrite) ){
            if( device->write(zero_array) < 0 )
                error = true;
            if( !device->flush() )
                error = true;

            device->close();

            if(error)
                KMessageBox::error(0, error_message);
        }
        else
            KMessageBox::error(0, error_message);

        return(true);
    }
    return(false);
}
