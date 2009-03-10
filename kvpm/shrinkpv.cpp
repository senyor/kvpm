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

#include <KLocale>
#include <KMessageBox>

#include <QtGui>

#include "processprogress.h"
#include "shrinkpv.h"


// Returns new pv size in bytes or 0 if no shrinking was done
// Takes new_size in bytes.


long long shrink_pv(QString path, long long new_size)
{

    QStringList arguments; 

    QString size_string;

    arguments << "pvresize" 
              << "--setphysicalvolumesize"
              << QString("%1B").arg( new_size )
              << path;

    ProcessProgress pv_shrink(arguments, i18n("Shrinking pv..."), true );
    if( pv_shrink.exitCode() )
        return 0;
    else
        return new_size;

}

