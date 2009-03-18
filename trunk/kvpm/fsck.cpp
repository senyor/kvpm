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

#include "processprogress.h"

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
