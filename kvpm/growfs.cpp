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

#include <QtGui>

#include "growfs.h"
#include "processprogress.h"
#include "fsck.h"

bool grow_fs(QString path){

    QStringList arguments, output;

    fsck( path );

    arguments << "resize2fs" 
              << path; 

    ProcessProgress fs_grow(arguments, i18n("Growing filesystem..."), true );
    output = fs_grow.programOutput();

    if ( fs_grow.exitCode() )
        return false;
    else
        return true;
}

