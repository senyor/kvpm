/*
 *
 * 
 * Copyright (C) 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#ifndef STORAGEMODEL_H
#define STORAGEMODEL_H

#include <QList>
#include <QObject>
#include <QAbstractItemModel>
#include <QVariant>

class StorageItem;
class StorageDevice;

class StorageModel : public QAbstractItemModel
{
    Q_OBJECT

    void setupModelData(QList<StorageDevice *> devices, StorageItem *parent);
    StorageItem *rootItem;

public:
    explicit StorageModel(QList<StorageDevice *> devices, QObject *parent = 0);
    ~StorageModel();
    
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

};

#endif
