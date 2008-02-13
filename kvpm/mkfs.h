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

bool make_fs(LogVol *LogicalVolume);
bool make_fs(StoragePartition *Partition);

class MkfsDialog : public KDialog
{
     QGroupBox *radio_box;
     QRadioButton *ext2, *ext3, *reiser, *jfs, *xfs, *vfat, *swap;
     QString type;
     QString path;

 public:
     MkfsDialog(QString DevicePath, QWidget *parent = 0);
     QStringList arguments();

};

#endif
