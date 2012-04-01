/*
 *
 *
 * Copyright (C) 2009, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "fsextend.h"

#include <sys/mount.h>
#include <errno.h>
#include <string.h>

#include <KLocale>
#include <KMessageBox>
#include <KTempDir>

#include <QtGui>

#include "executablefinder.h"
#include "fsck.h"
#include "mountentry.h"
#include "mounttables.h"
#include "processprogress.h"



bool do_temp_mount(const QString dev, const QString fs, const QString mp); // device path, filesystem and mount point
void do_temp_unmount(const QString mp);


bool fs_can_extend(const QString fs)
{
    QString executable;

    if (fs == "ext2" || fs == "ext3" || fs == "ext4" || fs == "reiserfs" || fs == "ntfs" || fs == "xfs") {

        if (fs == "ext2" || fs == "ext3" || fs == "ext4")
            executable = "resize2fs";
        else if (fs == "reiserfs")
            executable = "resize_reiserfs";
        else if (fs == "ntfs")
            executable = "ntfsresize";
        else if (fs == "xfs")
            executable = "xfs_growfs";

        if (!ExecutableFinder::getPath(executable).isEmpty())
            return true;
        else {
            KMessageBox::error(NULL, i18n("Executable: '%1' not found, this filesystem cannot be extended", executable));
            return false;
        }
    } else if (fs == "jfs")
        return true;
    else
        return false;
}

// dev is the full path to the device or volume, fs == filesystem, mps == mountpoints
bool fs_extend(const QString dev, const QString fs, const QStringList mps, const bool isLV)
{
    bool isMounted = true;
    QString mp;

    if (mps.isEmpty())
        isMounted = false;
    else
        mp = mps[0];

    const QByteArray dev_qba = dev.toLocal8Bit();
    const QByteArray  fs_qba = fs.toLocal8Bit();
    const QByteArray  mp_qba = mp.toLocal8Bit();
    QStringList args;
    unsigned long options = 0;

    if ((!isLV) && (isMounted)) {
        KMessageBox::error(0, i18n("only logical volumes may be mounted during resize, not partitions"));
        return false;
    }

    const QString err_msg = i18n("It appears that the filesystem on the device or volume was not extended. "
                                 "It will need to be extended before the additional space can be used.");

    if (fs == "ext2" || fs == "ext3" || fs == "ext4") {

        if (!isMounted) {
            if (!fsck(dev))
                return false;
        }

        args << "resize2fs" << dev;
        ProcessProgress fs_grow(args);

        if (fs_grow.exitCode()) {
            KMessageBox::error(0, err_msg);
            return false;
        } else
            return true;
    } else if (fs == "xfs") {

        if (isMounted) {

            args << "xfs_growfs" << mp;
            ProcessProgress fs_grow(args);

            if (fs_grow.exitCode()) {
                KMessageBox::error(0, err_msg);
                return false;
            } else
                return true;
        } else {

            KTempDir temp_mp;

            if (do_temp_mount(dev, fs, temp_mp.name())) {

                args << "xfs_growfs" << temp_mp.name();
                ProcessProgress fs_grow(args);
                do_temp_unmount(temp_mp.name());

                if (fs_grow.exitCode()) {
                    KMessageBox::error(0, err_msg);
                    return false;
                } else
                    return true;
            } else {
                KMessageBox::error(0, err_msg);
                return false;
            }
        }
    } else if (fs == "jfs") {

        options = MS_REMOUNT;

        if (isMounted) {

            if (mount(dev_qba.data(), mp_qba.data(), NULL, options, "resize")) {
                KMessageBox::error(0, i18n("Error number: %1 %2", errno, strerror(errno)));
                return false;
            } else
                return true;
        } else {

            KTempDir temp_mp;
            const QByteArray temp_mp_qba = temp_mp.name().toLocal8Bit();

            if (do_temp_mount(dev, fs, temp_mp.name())) {
                if (! mount(dev_qba.data(), temp_mp_qba.data(), NULL, options, "resize")) {
                    do_temp_unmount(temp_mp.name());
                    return true;
                } else {
                    KMessageBox::error(0, i18n("Error number: %1 %2", errno, strerror(errno)));
                    do_temp_unmount(temp_mp.name());
                }
            }
            return false;
        }
    } else if (fs == "reiserfs") {

        args << "resize_reiserfs" << "-fq" << dev;
        ProcessProgress fs_grow(args);

        if (fs_grow.exitCode()) {
            KMessageBox::error(0, err_msg);
            return false;
        } else
            return true;
    } else if (fs == "ntfs") {

        args << "ntfsresize"
             << "--no-progress-bar"
             << "--force"
             << dev;

        ProcessProgress fs_grow(args);

        if (fs_grow.exitCode()) {
            KMessageBox::error(0, err_msg);
            return false;
        } else
            return true;
    }

    return false;
}

bool do_temp_mount(const QString dev, const QString fs, const QString mp)
{
    const unsigned long options = 0;
    const QByteArray dev_qba = dev.toLocal8Bit();
    const QByteArray fs_qba  = fs.toLocal8Bit();
    const QByteArray mp_qba  = mp.toLocal8Bit();

    const int error = mount(dev_qba.data(), mp_qba.data(), fs_qba.data(), options, NULL);

    if (error) {
        KMessageBox::error(0, QString("Error number: %1 %2").arg(errno).arg(strerror(errno)));
        return false;
    }

    return true;
}

void do_temp_unmount(const QString mp)
{
    const QByteArray mp_qba = mp.toLocal8Bit();
    umount2(mp_qba, 0);
}

