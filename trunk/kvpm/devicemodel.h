/*
 *
 * 
 * Copyright (C) 2008, 2009, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef DEVICEMODEL_H
#define DEVICEMODEL_H

#include <QList>
#include <QAbstractItemModel>
#include <QVariant>
#include "masterlist.h"

class StorageDeviceItem
{

 public:
    StorageDeviceItem(const QList<QVariant> &data, const QList<QVariant> &dataAlternate, StorageDeviceItem *parent = 0);
    ~StorageDeviceItem();

    void appendChild(StorageDeviceItem *child);
    StorageDeviceItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    QVariant dataAlternate(int column) const;
    int row() const;
    StorageDeviceItem *parent();
    
 private:
    QList<StorageDeviceItem *> childItems;
    QList<QVariant> itemData;
    QList<QVariant> itemDataAlternate;
    StorageDeviceItem *parentItem;
};

class StorageDeviceModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit StorageDeviceModel(QList<StorageDevice *> devices, QObject *parent = 0);
    ~StorageDeviceModel();
    
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
			int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
		      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
    void setupModelData(QList<StorageDevice *> devices, StorageDeviceItem *parent);
    StorageDeviceItem *rootItem;
};

#endif
