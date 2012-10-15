/*
 *
 *
 * Copyright (C) 2012 Benjamin Scott   <benscott@nwlink.com>
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

#include <KConfigSkeleton>
#include <KGlobal>
#include <KLocale>
#include <KMessageBox>

#include <QDebug>



/* This class handles both the creation and extention of thin volumes 
   or snapshots since the processes are so similar. */


/* Create new thin volume*/

ThinCreateDialog::ThinCreateDialog(LogVol *const pool, QWidget *parent):
    LvCreateDialogBase(false, false, true, QString(""), pool->getName(), parent),
    m_vg(pool->getVg()),
    m_pool(pool)
{
    m_lv = NULL;
    m_extend = false;
    m_snapshot = false;
    m_bailout  = hasInitialErrors();
    m_fs_can_extend = false;

    setCaption("Create A New Thin Volume");

    initializeSizeSelector(m_vg->getExtentSize(), 0, getLargestVolume() / m_vg->getExtentSize());

    resetOkButton();
}


/* extend thin volume or take snapshot */

ThinCreateDialog::ThinCreateDialog(LogVol *const volume, const bool snapshot, QWidget *parent):
    LvCreateDialogBase(!snapshot, snapshot, true, volume->getName(), volume->getPoolName(), parent),
    m_snapshot(snapshot),
    m_extend(!snapshot),
    m_vg(volume->getVg()),
    m_lv(volume)
{
    m_bailout = hasInitialErrors();

    if (!snapshot) {
        setCaption("Extend Thin Volume");
        initializeSizeSelector(m_vg->getExtentSize(), m_lv->getExtents(), getLargestVolume() / m_vg->getExtentSize());
    } else {
        setCaption("Create Thin Snapshot");
    }

    resetOkButton();
}

void ThinCreateDialog::resetOkButton()
{
    enableButtonOk(isValid());
}

long long ThinCreateDialog::getLargestVolume()
{
    return 0x1000000000000;   // 256 TiB should be enough for anyone :-)
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
 
        args << QString("%1b").arg(getSelectorExtents() * m_vg->getExtentSize());
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
        args << "--thin" << "--snapshot" << m_lv->getFullName();
    } else {                                            // extend the current volume
        program_to_run = "lvextend";
        args << m_lv->getFullName();
    }

    args.prepend(program_to_run);

    return args;
}

// This function checks for problems that would make showing this dialog pointless
// returns true if there are problems and is used to set m_bailout.

bool ThinCreateDialog::hasInitialErrors()
{
    if (m_extend) {

        const QString warning_message = i18n("If this volume has a filesystem or data, it will need to be extended <b>separately</b>. "
                                             "Currently, only the ext2, ext3, ext4, xfs, jfs, ntfs and reiserfs file systems are "
                                             "supported for extension. The correct executable for extension must also be present. ");

        if (m_lv->isCowOrigin()) {
            if (m_lv->isOpen()) {
                KMessageBox::error(this, i18n("Snapshot origins cannot be extended while open or mounted"));
                return true;
            }

            const QList<LogVol *> snap_shots = m_lv->getSnapshots();
          
            for (int x = 0; x < snap_shots.size(); x++) {
                if (snap_shots[x]->isOpen() && !snap_shots[x]->isThinVolume()) {
                    KMessageBox::error(this, i18n("Volumes cannot be extended with open or mounted snapshots"));
                    return true;
                }
            }
        }

        m_fs_can_extend = fs_can_extend(m_lv->getFilesystem());

        if (!(m_fs_can_extend || m_lv->isCowSnap())) {
            if (KMessageBox::warningContinueCancel(NULL,
                                                   warning_message,
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

bool ThinCreateDialog::bailout()
{
    return m_bailout;
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
                KMessageBox::error(0, i18n("Volume activation failed, filesystem not extended"));
                return;
            }

            if (m_fs_can_extend)
                fs_extend(m_lv->getMapperPath(), fs, m_lv->getMountPoints(), true);

            return;
        } else {
            ProcessProgress extend_lv(args());
            if (!extend_lv.exitCode() && !m_lv->isCowSnap() && m_fs_can_extend)
                fs_extend(mapper_path, fs, m_lv->getMountPoints(), true);

            return;
        }
    }
}

