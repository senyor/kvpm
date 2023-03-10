/*
 *
 *
 * Copyright (C) 2012, 2013, 2014, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "thincreate.h"

#include "fsextend.h"
#include "logvol.h"
#include "misc.h"
#include "mountentry.h"
#include "processprogress.h"
#include "volgroup.h"

#include <math.h>

#include <KLocalizedString>
#include <KMessageBox>




/* This class handles both the creation and extention of thin volumes 
   or snapshots since the processes are so similar. */


/* Create new thin volume*/

ThinCreateDialog::ThinCreateDialog(LogVol *const pool, QWidget *parent):
    LvCreateDialogBase(pool->getVg(), -1, false, false, true, false, QString(""), pool->getName(), parent),
    m_pool(pool)
{
    m_lv = nullptr;
    m_extend = false;
    m_snapshot = false;
    m_fs_can_extend = false;
    const long long extent_size = getVg()->getExtentSize();
    const long long max_size = getLargestVolume();

    setCaption("Create A New Thin Volume");

    initializeSizeSelector(extent_size, 0, max_size / extent_size);
    
    setInfoLabels(THIN, 0, 0, max_size);

    connect(this, SIGNAL(extendFs()),
            this, SLOT(setMaxSize()));

    setMaxSize();
    resetOkButton();

    if (hasInitialErrors())
        preventExec();
}


/* extend thin volume or take snapshot */

ThinCreateDialog::ThinCreateDialog(LogVol *const volume, const bool snapshot, QWidget *parent):
    LvCreateDialogBase(volume->getVg(), fs_max_extend(volume->getMapperPath(), volume->getFilesystem(), volume->isMounted()), 
                       !snapshot, snapshot, true, false, volume->getName(), volume->getPoolName(), parent),
    m_snapshot(snapshot),
    m_extend(!snapshot),
    m_lv(volume)
{
    const long long extent_size = getVg()->getExtentSize();
    const long long max_size = getLargestVolume();

    if (!snapshot) {
        setCaption("Extend Thin Volume");
        initializeSizeSelector(extent_size, m_lv->getExtents(), max_size / extent_size);
    } else {
        setCaption("Create Thin Snapshot");
    }

    setInfoLabels(THIN, 0, 0, max_size);

    connect(this, SIGNAL(extendFs()),
            this, SLOT(setMaxSize()));

    setMaxSize();
    resetOkButton();

    if (hasInitialErrors())
        preventExec();
}

void ThinCreateDialog::setMaxSize()
{
    const long long max = getLargestVolume() / getVg()->getExtentSize();
    const long long maxfs = getMaxFsSize() / getVg()->getExtentSize();

    if (getExtendFs()) {
        if (max < maxfs)
            setSelectorMaxExtents(max);
        else
            setSelectorMaxExtents(maxfs);
    } else {
        setSelectorMaxExtents(max);
    }

    resetOkButton();
}

void ThinCreateDialog::resetOkButton()
{
    enableButtonOk(isValid());
}

long long ThinCreateDialog::getLargestVolume()
{
    // Current limitation of number of extents is 32bit unsigned

    return 0xFFFFFFFF * getVg()->getExtentSize();
}

/* Here we create a stringlist of arguments based on all
   the options that the user chose in the dialog. */

QStringList ThinCreateDialog::args()
{
    QString program_to_run;
    QStringList args;

    if (!getUdev())
        args << "--noudevsync";

    if (!m_snapshot) {
        if (m_extend)
            args << "--size";
        else
            args << "--virtualsize";
 
        args << QString("%1b").arg(getSelectorExtents() * getVg()->getExtentSize());
    }

    if (!m_extend) {
        if (!getName().isEmpty())
            args << "--name" << getName();

        if (!getTag().isEmpty())
            args << "--addtag" << getTag();

        if (getReadOnly())
            args << "--permission" << "r" ;
        else
            args << "--permission" << "rw" ;

        if (getPersistent()) {
            args << "--persistent" << "y";
            args << "--major" << getMajor();
            args << "--minor" << getMinor();
        }
    }

    if (!m_extend && !m_snapshot) {                     // create a thin volume
        program_to_run = "lvcreate";
        args << "--thin" << m_pool->getFullName();
    } else if (m_snapshot) {                            // create a thin snapshot
        program_to_run = "lvcreate";
        args << "--snapshot" << m_lv->getFullName();
    } else {                                            // extend the current volume
        program_to_run = "lvextend";
        args << m_lv->getFullName();
    }

    args.prepend(program_to_run);

    return args;
}

// This function checks for problems that would make showing this dialog pointless
// returns true if there are problems.

bool ThinCreateDialog::hasInitialErrors()
{
    if (getVg()->isPartial()) {
        if (m_extend)
            KMessageBox::sorry(this, i18n("Volumes can not be extended while physical volumes are missing"));
        else
            KMessageBox::sorry(this, i18n("Volumes can not be created while physical volumes are missing"));

        return true;
    }

    if (m_extend) {

        const QString warning1 = i18n("If this volume has a filesystem or data, it will need to be extended later "
                                      "by an appropriate tool. \n \n"
                                      "Currently, only the ext2, ext3, ext4, xfs, jfs, ntfs and reiserfs file systems are "
                                      "supported for extension. ");

        const QString warning2 = i18n("This filesystem seems to be as large as it can get, it will not be extended with the volume");

        const QString warning3 = i18n("ntfs cannot be extended while mounted. The filesystem will need to be "
                                      "extended later or unmounted before the volume is extended.");

        if (m_lv->isCowOrigin()) {
            if (m_lv->isOpen()) {
                KMessageBox::sorry(this, i18n("Snapshot origins cannot be extended while open or mounted"));
                return true;
            }

            const LvList snap_shots = m_lv->getSnapshots();
          
            for (int x = 0; x < snap_shots.size(); x++) {
                if (snap_shots[x]->isOpen() && !snap_shots[x]->isThinVolume()) {
                    KMessageBox::sorry(this, i18n("Volumes cannot be extended with open or mounted snapshots"));
                    return true;
                }
            }
        }

        m_fs_can_extend = fs_can_extend(m_lv->getFilesystem(), m_lv->isMounted());
        const long long maxfs = getMaxFsSize() / m_lv->getVg()->getExtentSize();
        const long long current = m_lv->getExtents(); 

        if ((m_lv->getFilesystem() == "ntfs") && m_lv->isMounted()) {
            if (KMessageBox::warningContinueCancel(nullptr,
                                                   warning3,
                                                   QString(),
                                                   KStandardGuiItem::cont(),
                                                   KStandardGuiItem::cancel(),
                                                   QString(),
                                                   KMessageBox::Dangerous) != KMessageBox::Continue) {
                return true;
            }
        } else if (!(m_fs_can_extend || m_lv->isCowSnap())) {
            if (KMessageBox::warningContinueCancel(nullptr,
                                                   warning1,
                                                   QString(),
                                                   KStandardGuiItem::cont(),
                                                   KStandardGuiItem::cancel(),
                                                   QString(),
                                                   KMessageBox::Dangerous) != KMessageBox::Continue) {
                return true;
            }
        } else if (current >= maxfs) {
            if (KMessageBox::warningContinueCancel(nullptr,
                                                   warning2,
                                                   QString(),
                                                   KStandardGuiItem::cont(),
                                                   KStandardGuiItem::cancel(),
                                                   QString(),
                                                   KMessageBox::Dangerous) != KMessageBox::Continue) {
                return true;
            }
        }


    }

    return false;
}

void ThinCreateDialog::commit()
{
    QStringList lvchange_args;
    hide();

    if (!m_extend) {
        ProcessProgress create_lv(args());
        return;
    } else {
        const QString mapper_path = m_lv->getMapperPath();
        const QString fs = m_lv->getFilesystem();

        if (m_lv->isCowOrigin()) {

            lvchange_args << "lvchange" << "-an" << mapper_path;
            ProcessProgress deactivate_lv(lvchange_args);
            if (deactivate_lv.exitCode()) {
                KMessageBox::error(0, i18n("Volume deactivation failed, volume not extended"));
                return;
            }

            ProcessProgress extend_origin(args());
            if (extend_origin.exitCode()) {
                KMessageBox::error(0, i18n("Volume extension failed"));
                return;
            }

            lvchange_args.clear();
            lvchange_args << "lvchange" << "-ay" << mapper_path;
            ProcessProgress activate_lv(lvchange_args);
            if (activate_lv.exitCode()) {
                if (getExtendFs())
                    KMessageBox::error(0, i18n("Volume activation failed, filesystem not extended"));
                else
                    KMessageBox::error(0, i18n("Volume activation failed"));
            } else if (getExtendFs()) {
                fs_extend(m_lv->getMapperPath(), fs, m_lv->getMountPoints(), true);
            }

            return;
        } else {
            ProcessProgress extend_lv(args());
            if (!extend_lv.exitCode() && getExtendFs())
                fs_extend(mapper_path, fs, m_lv->getMountPoints(), true);

            return;
        }
    }
}

