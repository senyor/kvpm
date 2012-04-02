/*
 *
 *
 * Copyright (C) 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "snapmerge.h"

#include "logvol.h"
#include "processprogress.h"

#include <KMessageBox>
#include <KLocale>

#include <QDebug>
#include <QStringList>



bool merge_snap(LogVol *const snapshot)
{
    QStringList args;
    const QString warning = i18n("Merge snapshot: <b>%1</b> with origin: <b>%2</b>?", snapshot->getName(), snapshot->getOrigin());

    if (KMessageBox::warningYesNo(NULL,
                                  warning,
                                  QString(),
                                  KStandardGuiItem::yes(),
                                  KStandardGuiItem::no(),
                                  QString(),
                                  KMessageBox::Dangerous) == KMessageBox::Yes) {

        args << "lvconvert"
             << "--merge"
             << "--background"
             << snapshot->getFullName();

        ProcessProgress merge(args);
        return true;
    } else {
        return false;
    }
}
