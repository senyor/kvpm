/*
 *
 * 
 * Copyright (C) 2009 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include <KLocale>
#include <KMessageBox>

#include <QtGui>

#include "fsblocksize.h"
#include "processprogress.h"


long get_fs_block_size(QString path){

    QStringList arguments, 
                output,
                temp_stringlist;

    QString block_string;

    arguments << "dumpe2fs" 
              << "-h" 
              << path;

    ProcessProgress blocksize_scan(arguments, i18n("Checking blocksize") );
    output = blocksize_scan.programOutput();

    temp_stringlist << output.filter("Block size", Qt::CaseInsensitive);

    if( temp_stringlist.size() ){
        block_string = temp_stringlist[0];
        block_string = block_string.remove( 0, block_string.indexOf(":") + 1 );
        block_string = block_string.simplified();
    }
    else
        return 0;

    return block_string.toLong();
}
