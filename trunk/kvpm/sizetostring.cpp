/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Klvm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <QtGui>


QString sizeToString(long long bytes)
{
    double size;
    
    size = (double)bytes;
    if(size < 1000)
	return QString("%1").arg(bytes);
    size /= 1024;
    if(size < 1000)
	return QString("%1 KB").arg(size, 0,'g', 3);
    size /= 1024;
    if(size < 1000)
	return QString("%1 MB").arg(size, 0, 'g', 3);
    size /= 1024;
    if(size < 1000)
	return QString("%1 GB").arg(size, 0,'g', 3);
    size /= 1024;
    return QString("%1 TB").arg(size, 0, 'g', 3);
}
