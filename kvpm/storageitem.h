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


#ifndef STORAGEITEM
#define STORAGEITEM

#include <QIcon>
#include <QList>
#include <QString>
#include <QVariant>


class StorageItem
{
    QList<StorageItem *> childItems;
    QList<QVariant> itemData;
    QList<QVariant> itemDataAlternate;
    QList<QVariant> itemDataIcon;
    QList<QVariant> itemDataToolTip;
    StorageItem *parentItem;

 public:
    StorageItem(const QList<QVariant> &data, const QList<QVariant> &dataAlternate, StorageItem *parent = 0);
    ~StorageItem();

    void appendChild(StorageItem *child);
    StorageItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    QVariant dataIcon(int column) const;
    QVariant dataToolTip(int column) const;
    QVariant dataAlternate(int column) const;
    int row() const;
    StorageItem *parent();
    void setIcon(int column, const QIcon & icon);
    void setToolTip(int column, QString tip);
    
 };

#endif
