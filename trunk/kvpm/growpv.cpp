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
#include "growpv.h"


bool grow_pv(QString path)
{

    QStringList arguments; 

    QString size_string;

    arguments << "pvresize" 
              << path;

    ProcessProgress pv_grow( arguments, i18n("Growing pv..."), true );

    if( pv_grow.exitCode() )
        return false;
    else
        return true;

}

