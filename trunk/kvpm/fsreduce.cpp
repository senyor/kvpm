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

#include "fsreduce.h"

#include <KLocale>
#include <KMessageBox>

#include <QtGui>

#include "executablefinder.h"
#include "fsblocksize.h"
#include "fsck.h"
#include "processprogress.h"



bool fs_can_reduce(const QString fs)
{
    const QString executable = "resize2fs";

    if (fs == "ext2" || fs == "ext3" || fs == "ext4") {
        if (ExecutableFinder::getPath(executable).isEmpty()) {
            KMessageBox::error(NULL, i18n("Executable: '%1' not found, this filesystem cannot be reduced", executable));
            return false;
        } else
            return true;
    } else
        return false;
}


// Returns new fs size in bytes or 0 if no shrinking was done
// Returns -1 if fs isn't one of ext2, ext3 or ext4 (not shrinkable)
// Takes new_size in bytes.

long long fs_reduce(const QString path, const long long new_size, const QString fs)
{

    QStringList arguments,
                output,
                success_stringlist,
                nothing_stringlist,
                nospace_stringlist;

    QString size_string;

    if (!fs_can_reduce(fs))
        return -1;

    if (! fsck(path))
        return 0;

    const long block_size = get_fs_block_size(path);

    if (block_size <= 0) {
        KMessageBox::error(0, i18n("Shrink failed: could not determine filesystem block size"));
        return 0 ;
    }

    arguments << "resize2fs"
              << path
              << QString("%1K").arg(new_size / 1024);

    ProcessProgress fs_shrink(arguments);
    output = fs_shrink.programOutputAll();

    success_stringlist = output.filter("is now");      // it worked
    nothing_stringlist = output.filter("is already");  // already a reduced fs, nothing to do!
    nospace_stringlist = output.filter("space left");

    if (success_stringlist.size() > 0) { // Try to shrink the desired amount

        size_string = success_stringlist[0];
        size_string = size_string.remove(0, size_string.indexOf("now") + 3);
        size_string.truncate(size_string.indexOf("blocks"));
        size_string = size_string.simplified();
        return size_string.toLongLong() * block_size;

    } else if (nothing_stringlist.size() > 0) {

        size_string = nothing_stringlist[0];
        size_string = size_string.remove(0, size_string.indexOf("already") + 7);
        size_string.truncate(size_string.indexOf("blocks"));
        size_string = size_string.simplified();
        return size_string.toLongLong() * block_size;

    } else if (nospace_stringlist.size() > 0) { // Couldn't shrink that much but try again with -M

        arguments.clear();
        success_stringlist.clear();

        arguments << "resize2fs"
                  << "-M"
                  << path;

        ProcessProgress fs_shrink(arguments);
        output = fs_shrink.programOutput();
        success_stringlist = output.filter("is now");

        if (success_stringlist.size() > 0) {

            size_string = success_stringlist[0];
            size_string = size_string.remove(0, size_string.indexOf("now") + 3);
            size_string.truncate(size_string.indexOf("blocks"));
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

    QStringList arguments,
                output;

    QString size_string;

    if (fs_can_reduce(fs)) {
        arguments << "resize2fs" << "-P" << path;

        long block_size = get_fs_block_size(path);
        if (block_size) {                        // if blocksize failed skip this part

            ProcessProgress fs_scan(arguments);
            output = fs_scan.programOutput();

            if (output.size() > 0 && fs_scan.exitCode() == 0) {

                size_string = output[0];

                if (size_string.contains("Estimated", Qt::CaseInsensitive)) {
                    size_string = size_string.remove(0, size_string.indexOf(":") + 1);
                    size_string = size_string.simplified();
                    return size_string.toLongLong() * block_size;
                } else
                    return 0;
            }
        }

        return 0;
    }

    return 0;
}

