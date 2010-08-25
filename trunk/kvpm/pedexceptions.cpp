/*
 *
 * 
 * Copyright (C) 2010 Benjamin Scott   <benscott@nwlink.com>
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

#include <QtGui>

#include "pedexceptions.h"


PedExceptionOption my_handler(PedException *exception)
{
    if (exception->type == PED_EXCEPTION_INFORMATION)
        KMessageBox::information( 0, exception->message );
    else
        KMessageBox::error( 0, exception->message );

    return PED_EXCEPTION_UNHANDLED;
}

