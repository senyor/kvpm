/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "removemirrorleg.h"

#include <KMessageBox>
#include <KLocale>

#include <QDebug>
#include <QStringList>

#include "logvol.h"
#include "processprogress.h"
#include "volgroup.h"


bool remove_mirror_leg(LogVol *mirrorLeg)
{
    const QString warning = i18n("Remove mirror leg: %1 ?", mirrorLeg->getName());

    if (KMessageBox::warningYesNo(nullptr,
                                  warning,
                                  QString(),
                                  KStandardGuiItem::yes(),
                                  KStandardGuiItem::no(),
                                  QString(),
                                  KMessageBox::Dangerous) == KMessageBox::Yes) {

        QStringList args;

        args << "lvconvert"
             << "--mirrors"
             << QString("-1")
             << mirrorLeg->getParentMirror()->getFullName()
             << mirrorLeg->getPvNamesAll();

        ProcessProgress remove(args);
        return true;
    } else {
        return false;
    }
}
