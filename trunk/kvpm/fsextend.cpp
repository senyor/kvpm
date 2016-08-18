/*
 *
 *
 * Copyright (C) 2009, 2011, 2012, 2013, 2014, 2016 Benjamin Scott   <benscott@nwlink.com>
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

#include <QApplication>
#include <KLocalizedString>
#include <KMessageBox>

#include <QTemporaryDir>

#include "executablefinder.h"
#include "fsck.h"
#include "fsblocksize.h"
#include "mountentry.h"
#include "mounttables.h"
#include "processprogress.h"
#include "progressbox.h"
#include "topwindow.h"



bool do_temp_mount(const QByteArray dev, const QByteArray fs, const QByteArray mp); // device path, filesystem and mount point
void do_temp_unmount(const QByteArray mp);
bool resize_jfs_mount(const QByteArray dev, const QByteArray mp);


bool fs_can_extend(const QString fs, const bool mounted)
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

        if (mounted && ("ntfs" == fs)) {
            return false;
        } else if (ExecutableFinder::getPath(executable).isEmpty()) {
            KMessageBox::sorry(nullptr, i18n("Executable: '%1' not found, this filesystem cannot be extended", executable));
            return false;
        } else {
            return true;
        }
    } else if (fs == "jfs") {
        return true;
    }

    return false;
}

// The largest size the filesystem can extend to in bytes or -1 if the fs
// type can't be extended.
long long fs_max_extend(const QString dev, const QString fs, const bool mounted)
{
    long long max = -1;
    
    if (fs_can_extend(fs, mounted)) {

        if (fs == "ext2" || fs == "ext3") {
            const long bs = get_fs_block_size(dev);

            if (bs == 1024)
                max = 0x20000000000 - 1;   // 2 TiB
            else if (bs == 2048)
                max = 0x80000000000 - 1;   // 8 TiB
            else
                max = 0x100000000000 - 1;  // 16 TiB
            
 //     } else if (fs == "ext4") {
 //         max = 0x1000000000000000 - 1;  // 1 EiB ext4 theoretical limit
        } else if (fs == "ext4") {
            max = 0x100000000000 - 1;      // 16 TiB seems to be the current limit for the ext fs resize tools
        } else if (fs == "reiserfs") {
            max = 0x100000000000 - 1;      // 16 TiB
        } else if (fs == "ntfs") {
            max = 0x100000000000 - 4096;   // 16 TiB -- assumes small cluster size
        } else if (fs == "xfs") {
            max = 0x8000000000000000 - 1;  // 8 EiB
        } else if (fs == "jfs") { 
            max = 0x2000000000000 - 1;     // 512 TiB -- assumes 512 Bytes block size
        } else { 
            max = -1;
        }
    }

    return max;
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

    if (isMounted) {
        if (!isLV) {
            KMessageBox::sorry(nullptr, i18n("Only logical volumes may be mounted during resize, not partitions."));
            return false;
        } else if (fs != "ext2" && fs != "ext3" && fs != "ext4" && fs != "reiserfs" && fs != "jfs" && fs != "xfs") {
            KMessageBox::sorry(nullptr, i18n("Filesystem '%1' can not be extended while mounted.", fs));
            return false;
        }
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
            KMessageBox::sorry(nullptr, err_msg);
            return false;
        } else {
            return true;
        }
    } else if (fs == "xfs") {
        if (isMounted) {

            args << "xfs_growfs" << mp;
            ProcessProgress fs_grow(args);

            if (fs_grow.exitCode()) {
                KMessageBox::error(nullptr, err_msg);
                return false;
            } else {
                return true;
            }
        } else {
            QTemporaryDir temp_mp;
            const QByteArray temp_mp_qba = temp_mp.path().toLocal8Bit();

            if (do_temp_mount(dev_qba, fs_qba, temp_mp_qba)) {

                args << "xfs_growfs" << temp_mp_qba;
                ProcessProgress fs_grow(args);
                do_temp_unmount(temp_mp_qba);

                if (fs_grow.exitCode()) {
                    KMessageBox::error(nullptr, err_msg);
                    return false;
                } else {
                    return true;
                }
            } else {
                KMessageBox::error(nullptr, err_msg);
                return false;
            }
        }
    } else if (fs == "jfs") {
        bool success = false; 
        if (isMounted) {
            success = resize_jfs_mount(dev_qba, mp_qba);
        } else {
            QTemporaryDir temp_mp;
            const QByteArray temp_mp_qba = temp_mp.path().toLocal8Bit();

            if (do_temp_mount(dev_qba, fs_qba, temp_mp_qba)) {
                success = resize_jfs_mount(dev_qba, temp_mp_qba);
                do_temp_unmount(temp_mp_qba);
            }
        }

        return success;
    } else if (fs == "reiserfs") {

        args << "resize_reiserfs" 
             << "-fq" 
             << dev;

        ProcessProgress fs_grow(args);

        if (fs_grow.exitCode()) {
            KMessageBox::error(nullptr, err_msg);
            return false;
        } else {
            return true;
        }
    } else if (fs == "ntfs") {

        args << "ntfsresize"
             << "--no-progress-bar"
             << "--force"
             << dev;

        ProcessProgress fs_grow(args);

        if (fs_grow.exitCode()) {
            KMessageBox::error(nullptr, err_msg);
            return false;
        } else {
            return true;
        }
    }

    return false;
}

bool do_temp_mount(const QByteArray dev, const QByteArray fs, const QByteArray mp)
{
    const unsigned long options = 0;
    int error = 0;

    if ("xfs" == fs)
        error = mount(dev.data(), mp.data(), fs.data(), options, "nouuid");
    else
        error = mount(dev.data(), mp.data(), fs.data(), options, nullptr);

    if (error) {
        KMessageBox::error(nullptr, QString("%1: Error number: %2 %3").arg("mount()").arg(errno).arg(strerror(errno)));
        return false;
    }

    return true;
}

void do_temp_unmount(const QByteArray mp)
{
    if(umount2(mp, 0))
        KMessageBox::error(nullptr, QString("%1: Error number: %2 %3").arg("umount()").arg(errno).arg(strerror(errno)));
}

bool resize_jfs_mount(const QByteArray dev, const QByteArray mp)
{
    bool success = false;

    TopWindow::getProgressBox()->setRange(0, 2);
    TopWindow::getProgressBox()->setValue(1);
    TopWindow::getProgressBox()->setText("jfs resize");
    qApp->setOverrideCursor(Qt::WaitCursor);

    if (mount(dev.data(), mp.data(), "jfs", MS_REMOUNT, "resize")) {  
        qApp->restoreOverrideCursor();
        KMessageBox::error(nullptr, QString("%1: Error number: %2 %3").arg("mount()").arg(errno).arg(strerror(errno)));
        success = false;
    } else {
        qApp->restoreOverrideCursor();
        success = true;
    }

    TopWindow::getProgressBox()->reset();
    return success;
}



    

    

    

    
