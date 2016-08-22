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


#include "pvextend.h"

#include <KMessageBox>

#include <QStringList>

#include "processprogress.h"
#include "storagepartition.h"


bool pv_extend(QString path)
{
    QStringList arguments;

    arguments << "pvresize"
              << path;

    ProcessProgress pv_grow(arguments);

    if (pv_grow.exitCode())
        return false;
    else
        return true;
}

