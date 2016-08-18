/*
 *
 *
 * Copyright (C) 2013, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "resync.h"

#include <KLocalizedString>
#include <KMessageBox>

#include <QStringList>

#include "logvol.h"
#include "processprogress.h"


bool resync(LogVol *const lv)
{
    if (lv->isOpen() || lv->isMounted()) {

        const QString error = i18n("Volume <b>%1</b> is in use or mounted and cannot "
                                   "be deactivated for re-synchronization" , lv->getName());

        KMessageBox::sorry(nullptr, error);

        return false;
    }

    const QString warning = i18n("Really re-synchronize <b>%1?</b> " 
                                 "That could take a long time.",
                                 lv->getName());

    if (KMessageBox::warningYesNo(nullptr,
                                  warning,
                                  QString(),
                                  KStandardGuiItem::yes(),
                                  KStandardGuiItem::no(),
                                  QString(),
                                  KMessageBox::Dangerous) == KMessageBox::Yes) {

        QStringList args;

        args << "lvchange"
             << "--yes"
             << "--resync"
             << lv->getFullName();

        ProcessProgress resync(args);

        return true;
    } else {
        return false;
    }
}
