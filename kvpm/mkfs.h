/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef MKFS_H
#define MKFS_H

#include <KDialog>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QStringList>

class VolGroup;
class LogVol;
class StoragePartition;

bool make_fs(LogVol *logicalVolume);
bool make_fs(StoragePartition *partition);

class MkfsDialog : public KDialog
{
     QGroupBox *radio_box;
     QRadioButton *ext2, *ext3, *reiser, *jfs, *xfs, *vfat, *swap;

     QString m_path;

 public:
     MkfsDialog(QString devicePath, QWidget *parent = 0);
     QStringList arguments();

};

#endif
