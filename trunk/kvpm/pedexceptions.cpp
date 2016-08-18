/*
 *
 *
 * Copyright (C) 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
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

#include <QApplication>
#include <KMessageBox>

#include <QString>


/*   If the error being handeled has a question we put up a 
     Yes/No warning box. Otherwise we put up a "sorry dialog" 
     for warnings and errors or an "error dialog" for bugs, fatal 
     errors and missing features. Generally a "Yes" choice will 
     tell Ped to fix the problem while "No" will ignore it.  */

PedExceptionOption my_handler(PedException *exception)
{
    QString message(exception->message);

    if (message.contains("unrecognised disk label"))
        return PED_EXCEPTION_CANCEL;

    qApp->restoreOverrideCursor();                   // reset the cursor to not-busy

    const PedExceptionOption options = exception->options;

    if (exception->type == PED_EXCEPTION_INFORMATION){
        KMessageBox::information(nullptr, message);
        return PED_EXCEPTION_CANCEL;
    } else if (exception->type == PED_EXCEPTION_WARNING || exception->type == PED_EXCEPTION_ERROR) {
        if (options & (PED_EXCEPTION_FIX | PED_EXCEPTION_YES)) {
            if (KMessageBox::warningYesNo(nullptr, message) == KMessageBox::Yes) {
                if (options & PED_EXCEPTION_FIX)
                    return PED_EXCEPTION_FIX;
                else
                    return PED_EXCEPTION_YES;
            } else {
                if (options & PED_EXCEPTION_IGNORE)
                    return PED_EXCEPTION_IGNORE;
                else if (options & PED_EXCEPTION_NO)
                    return PED_EXCEPTION_NO;
                else if (options & PED_EXCEPTION_OK)
                    return PED_EXCEPTION_OK;
                else
                    return PED_EXCEPTION_CANCEL;
            }
        } else {
            KMessageBox::sorry(nullptr, message);
        } 
    } else {
        KMessageBox::error(nullptr, message);
        return PED_EXCEPTION_CANCEL;
    }

    return PED_EXCEPTION_CANCEL;
}
