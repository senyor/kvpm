/*
 *
 *
 * Copyright (C) 2008, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "fsprobe.h"

#include <blkid/blkid.h>

#include <QtGui>


#define BLKID_EMPTY_CACHE       "/dev/null"

QString fsprobe_getfstype2(const QString devicePath)
{
    static blkid_cache blkid2;
    const QByteArray path = devicePath.toLocal8Bit();
    blkid2 = NULL;

    if (blkid_get_cache(&blkid2, BLKID_EMPTY_CACHE) < 0) {
        return QString();
    } else {
        char *tag = blkid_get_tag_value(blkid2, "TYPE", path.data());
        QString type(tag);
        free(tag);
        blkid_put_cache(blkid2);
        return type;
    }
}

QString fsprobe_getfsuuid(const QString devicePath)
{
    static blkid_cache blkid2;
    const QByteArray path = devicePath.toLocal8Bit();
    blkid2 = NULL;

    if (blkid_get_cache(&blkid2, BLKID_EMPTY_CACHE) < 0) {
        return QString();
    } else {
        char *tag = blkid_get_tag_value(blkid2, "UUID", path.data());
        QString uuid(tag);
        free(tag);
        blkid_put_cache(blkid2);
        return uuid;
    }
}


QString fsprobe_getfslabel(const QString devicePath)
{
    static blkid_cache blkid2;
    const QByteArray path = devicePath.toLocal8Bit();
    blkid2 = NULL;

    if (blkid_get_cache(&blkid2, BLKID_EMPTY_CACHE) < 0) {
        return QString();
    } else {
        char *tag = blkid_get_tag_value(blkid2, "LABEL", path.data());
        QString label(tag);
        free(tag);
        blkid_put_cache(blkid2);
        return label;
    }
}
