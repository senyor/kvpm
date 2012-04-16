/*
 *
 *
 * Copyright (C) 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "pedexceptions.h"

#include <parted/parted.h>

#include <KApplication>
#include <KMessageBox>

#include <QDebug>
#include <QString>



PedExceptionOption my_handler(PedException *exception)
{
    QString error_message;
    qApp->restoreOverrideCursor(); // reset the cursor to not-busy

    if (exception->type == PED_EXCEPTION_INFORMATION)
        KMessageBox::information(0, exception->message);
    else {
        error_message = QString(exception->message);
        if (!error_message.contains("unrecognised disk label"))
            KMessageBox::error(0, exception->message);
    }

    return PED_EXCEPTION_UNHANDLED;
}
