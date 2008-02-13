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
#ifndef PARTITIONPROPERTIESSTACK_H
#define PARTITIONPROPERTIESSTACK_H

#include <QList>
#include <QModelIndex>
#include <QStackedWidget>
#include <QStringList>

class StoragePartition;
class StorageDevice;

class PartitionPropertiesStack : public QStackedWidget
{
Q_OBJECT

    QStringList partition_path_list;  // full path of each partition on the stack, same order
    int partition_count;
 
 public:
     PartitionPropertiesStack(QList<StorageDevice *> Devices, QWidget *parent = 0);

 public slots:
     void changePartitionStackIndex(QModelIndex Index);
};

#endif
