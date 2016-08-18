/*
 *
 *
 * Copyright (C) 2011, 2012, 2014, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "maxfs.h"

#include "fsextend.h"
#include "logvol.h"
#include "processprogress.h"
#include "physvol.h"
#include "pvextend.h"
#include "storagedevice.h"
#include "storagepartition.h"

#include <KLocalizedString>
#include <KMessageBox>

#include <QString>


bool max_fs(LogVol *const logicalVolume)
{
    const QString path = logicalVolume->getMapperPath();
    const QString fs   = logicalVolume->getFilesystem();
    QString full_name  = logicalVolume->getFullName();
    full_name.remove('[').remove(']');

    const QString warning = i18n("Extend the filesystem on: %1 to fill the entire volume?", "<b>" + full_name + "</b>");
    const QString error_message1 = i18n("'ntfs' filesystem must be unmounted before extending.");
    const QString error_message2 = i18n("Extending is only supported for ext2, ext3, ext4, jfs, xfs, ntfs and Reiserfs. "
                                        "The correct executables for file system extension must also be present.");

    if (logicalVolume->isMounted() && ("ntfs" == fs)) {
        KMessageBox::sorry(nullptr, error_message1);
        return false;
    } else if (!fs_can_extend(fs, logicalVolume->isMounted())) {
        KMessageBox::sorry(nullptr, error_message2);
        return false;
    }

    if (KMessageBox::warningYesNo(nullptr,
                                  warning,
                                  QString(),
                                  KStandardGuiItem::yes(),
                                  KStandardGuiItem::no(),
                                  QString(),
                                  KMessageBox::Dangerous) == KMessageBox::Yes) {
        return fs_extend(path, fs, logicalVolume->getMountPoints(), true);
    } else {
        return false;
    }
}

bool max_fs(StoragePartition *const partition)
{
    const QString path = partition->getName();
    const QString fs = partition->getFilesystem();
    const QString error_fs = i18n("Filesystem extending is only supported for ext2, ext3, ext4, jfs, xfs, ntfs and Reiserfs. "
                                  "Physical volumes can also be extended. "
                                  "The correct executables for file system extension must be present");
    const QString error_active = i18n("Physical volumes cannot be extended if they contain any active logical volumes");

    QString message;

    if (partition->isPhysicalVolume()) {
        if (partition->getPhysicalVolume()->isActive()) {
            KMessageBox::sorry(nullptr, error_active);
            return false;
        } else {
            message = i18n("Extend the physical volume on: %1 to fill the entire partition?", "<b>" + path + "</b>");
        }
    } else {
        message = i18n("Extend the filesystem on: %1 to fill the entire partition?", "<b>" + path + "</b>");
    }

    if (!(fs_can_extend(fs, partition->isMounted()) || partition->isPhysicalVolume())) {
        KMessageBox::sorry(nullptr, error_fs);
        return false;
    }

    if (KMessageBox::warningYesNo(nullptr,
                                  message,
                                  QString(),
                                  KStandardGuiItem::yes(),
                                  KStandardGuiItem::no(),
                                  QString(),
                                  KMessageBox::Dangerous) == KMessageBox::Yes) {
        if (partition->isPhysicalVolume()) {
            return pv_extend(path);
        } else {
            return fs_extend(path, fs, partition->getMountPoints(), false);
        }
    }

    return false;
}

bool max_fs(StorageDevice *const device)
{
    const QString path = device->getName();
    const QString warning = i18n("Extend the physical volume on: %1 to fill the entire partition?", "<b>" + path + "</b>");
    const QString error_active = i18n("Physical volumes cannot be extended if they contain any active logical volumes");

    if (!device->isPhysicalVolume()) {
        return false;
    } else if (device->getPhysicalVolume()->isActive()) {
        KMessageBox::sorry(nullptr, error_active);
        return false;
    }

    if (KMessageBox::warningYesNo(nullptr,
                                  warning,
                                  QString(),
                                  KStandardGuiItem::yes(),
                                  KStandardGuiItem::no(),
                                  QString(),
                                  KMessageBox::Dangerous) == KMessageBox::Yes) {
        return pv_extend(path);
    }

    return false;
}
