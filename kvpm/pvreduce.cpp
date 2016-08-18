/*
 *
 *
 * Copyright (C) 2009, 2011, 2012, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "pvreduce.h"

#include <QStringList>


#include "processprogress.h"



// Returns new pv size in bytes or 0 if no shrinking was done
// Takes new_size in bytes.

long long pv_reduce(QString path, long long new_size)
{
    QStringList arguments;
    QString size_string;

    arguments << "pvresize"
              << "--setphysicalvolumesize"
              << QString("%1m").arg(new_size / (1024 * 1024))
              << path;

    ProcessProgress pv_shrink(arguments);

    if (pv_shrink.exitCode())
        return 0;
    else
        return new_size;

}

