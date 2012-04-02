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

#include "partremove.h"

#include "storagepartition.h"

#include <parted/parted.h>

#include <KLocale>
#include <KMessageBox>

#include <QDebug>


bool remove_partition(StoragePartition *const partition)
{
    PedPartition *const ped_partition = partition->getPedPartition();

    if (partition->getType() != "extended") {
        const QString warning = i18n("Remove partition: <b>%1?</b> Any data on that partition will be lost.", partition->getName());

        if (KMessageBox::warningYesNo(NULL,
                                      warning,
                                      QString(),
                                      KStandardGuiItem::yes(),
                                      KStandardGuiItem::no(),
                                      QString(),
                                      KMessageBox::Dangerous) == KMessageBox::Yes) {

            if (ped_disk_delete_partition(ped_partition->disk, ped_partition))
                ped_disk_commit(ped_partition->disk);

            return true;
        }
    } else {
        const QString question = i18n("Remove partition: <b>%1?</b>", partition->getName());

        if (KMessageBox::questionYesNo(0, question) == KMessageBox::Yes) {

            if (ped_disk_delete_partition(ped_partition->disk, ped_partition))
                ped_disk_commit(ped_partition->disk);

            return true;
        }
    }

    return false;
}
