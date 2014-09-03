/*
 *
 *
 * Copyright (C) 2009, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "fsreduce.h"

#include <KLocale>
#include <KMessageBox>

#include <QDebug>
#include <QString>

#include "executablefinder.h"
#include "fsblocksize.h"
#include "fsck.h"
#include "processprogress.h"



bool fs_can_reduce(const QString fs)
{
    const QString executable = "resize2fs";

    if (fs == "ext2" || fs == "ext3" || fs == "ext4") {
        if (ExecutableFinder::getPath(executable).isEmpty()) {
            KMessageBox::error(nullptr, i18n("Executable: '%1' not found, this filesystem cannot be reduced", executable));
            return false;
        } else {
            return true;
        }
    } else {
        return false;
    }
}

// Returns new fs size in bytes or 0 if no shrinking was done
// Returns -1 if fs isn't one of ext2, ext3 or ext4 (not shrinkable)
// Takes new_size in bytes.

long long fs_reduce(const QString path, const long long new_size, const QString fs)
{
    if (!fs_can_reduce(fs))
        return -1;

    if (!fsck(path))
        return 0;

    const long block_size = get_fs_block_size(path);

    if (block_size <= 0) {
        KMessageBox::error(nullptr, i18n("Shrink failed: could not determine filesystem block size"));
        return 0 ;
    }

    QStringList args = QStringList() << "resize2fs"
                                     << path
                                     << QString("%1K").arg(new_size / 1024);
    ProcessProgress fs_shrink(args);
    QStringList output = fs_shrink.programOutputAll();

    QStringList success_stringlist = output.filter("is now");      // it worked
    QStringList nothing_stringlist = output.filter("is already");  // already a reduced fs, nothing to do!
    QStringList nospace_stringlist;
    nospace_stringlist << output.filter("space left");    // fs won't shrink that much -- old message
    nospace_stringlist << output.filter("than minimum");  // fs won't shrink that much -- new message

    QString size_string;

    if (success_stringlist.size() > 0) { // Try to shrink the desired amount

        size_string = success_stringlist[0];
        size_string = size_string.remove(0, size_string.indexOf("now") + 3);
        size_string.truncate(size_string.indexOf("blocks"));
        if (size_string.indexOf("(") > -1)
            size_string.truncate(size_string.indexOf("("));
        size_string = size_string.simplified();
        return size_string.toLongLong() * block_size;

    } else if (nothing_stringlist.size() > 0) {

        size_string = nothing_stringlist[0];
        size_string = size_string.remove(0, size_string.indexOf("already") + 7);
        size_string.truncate(size_string.indexOf("blocks"));
        if (size_string.indexOf("(") > -1)
            size_string.truncate(size_string.indexOf("("));
        size_string = size_string.simplified();
        return size_string.toLongLong() * block_size;

    } else if (nospace_stringlist.size() > 0) { // Couldn't shrink that much but try again with -M
        KMessageBox::information(nullptr, i18n("Now trying to shrink the filesystem to its minimum size"));

        args.clear();
        success_stringlist.clear();

        args << "resize2fs"
                  << "-M"
                  << path;

        ProcessProgress fs_shrink(args);
        output = fs_shrink.programOutput();
        success_stringlist = output.filter("is now");

        if (success_stringlist.size() > 0) {

            size_string = success_stringlist[0];
            size_string = size_string.remove(0, size_string.indexOf("now") + 3);
            size_string.truncate(size_string.indexOf("blocks"));
            if (size_string.indexOf("(") > -1)
                size_string.truncate(size_string.indexOf("("));
            size_string = size_string.simplified();
            return size_string.toLongLong() * block_size;
        }
    }
    // Give up and do nothing

    return 0;

}

// Returns estimated minimum size of filesystem after shrinking, in bytes
// Returns 0 on failure

long long get_min_fs_size(const QString path, const QString fs)
{
    long long size = 0;
    if (fs_can_reduce(fs)) {

        const long block_size = get_fs_block_size(path);
        if (block_size) {                        // if blocksize failed skip this part
            
            const QStringList args = QStringList() << "resize2fs" << "-P" << path;
            ProcessProgress fs_scan(args);
            const QStringList output = fs_scan.programOutput();
            
            if (output.size() > 0 && fs_scan.exitCode() == 0) {
                
                QString size_string = output[0];
                if (size_string.contains("Estimated", Qt::CaseInsensitive)) {
                    size_string = size_string.remove(0, size_string.indexOf(":") + 1);
                    size_string = size_string.simplified();
                    size = size_string.toLongLong() * block_size;
                }
            }
        }
    }

    return size;
}
