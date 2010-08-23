/*
 *
 * 
 * Copyright (C) 2010 Benjamin Scott   <benscott@nwlink.com>
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

#include "fsdata.h"
#include "processprogress.h"
#include "misc.h"

FSData *get_fs_data(QString path){

    QStringList arguments, 
                output,
                temp_stringlist;

    QString block_string;

    long long block_size = 0;
    long long total_blocks = 0;
    long long used_blocks = 0;

    arguments << "dumpe2fs" << "-h" << path;

    ProcessProgress size_scan(arguments, i18n("Checking size"), false );
    output = size_scan.programOutput();

    temp_stringlist << output.filter("Block size", Qt::CaseInsensitive);

    if( temp_stringlist.size() ){
        block_string = temp_stringlist[0];
        block_string = block_string.remove( 0, block_string.indexOf(":") + 1 );
        block_string = block_string.simplified();
        block_size = block_string.toLong();
    }

    temp_stringlist.clear();

    temp_stringlist << output.filter("Block count", Qt::CaseInsensitive);
    if( temp_stringlist.size() && block_size > 0 ){
        block_string = temp_stringlist[0];
        block_string = block_string.remove( 0, block_string.indexOf(":") + 1 );
        block_string = block_string.simplified();
        total_blocks = block_string.toLong();
    }

    temp_stringlist.clear();

    temp_stringlist << output.filter("Free blocks", Qt::CaseInsensitive);
    if( temp_stringlist.size() && total_blocks > 0 ){
        block_string = temp_stringlist[0];
        block_string = block_string.remove( 0, block_string.indexOf(":") + 1 );
        block_string = block_string.simplified();
        used_blocks  = total_blocks - block_string.toLong();
    }

    FSData *fs_data = new FSData();
    fs_data->size = -1;
    fs_data->used = -1;
    fs_data->block_size = -1;

    if( block_size > 0 && total_blocks > 0 ){
        fs_data->size = block_size * total_blocks;
        fs_data->used = block_size * used_blocks;
        fs_data->block_size = block_size;
    }

    return fs_data;
}
