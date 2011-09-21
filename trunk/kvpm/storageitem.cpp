/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "storageitem.h"

#include <KIcon>
#include <QtGui>


StorageItem::StorageItem(const QList<QVariant> &data, 
                         const QList<QVariant> &dataAlternate, 
                         StorageItem *parent)
{
    parentItem = parent;
    itemData = data;
    itemDataAlternate = dataAlternate;

    for(int x = itemData.size() - 1; x >= 0; x--) // makes the icon list as long as the main data list 
        itemDataIcon.append( QVariant() );
}

StorageItem::~StorageItem()
{
    qDeleteAll(childItems);
}

void StorageItem::appendChild(StorageItem *item)
{
    childItems.append(item);
}

StorageItem *StorageItem::child(int row)
{
    return childItems[row];
}

int StorageItem::childCount() const
{
    return childItems.count();
}

int StorageItem::row() const
{
    if(parentItem)
	return parentItem->childItems.indexOf(const_cast<StorageItem *>(this));
    
    return 0;
}

int StorageItem::columnCount() const
{
    return itemData.count();
}

QVariant StorageItem::data(int column) const
{
    return itemData.value(column);
}

QVariant StorageItem::dataIcon(int column) const
{
    if( column < itemDataIcon.size() )
        return itemDataIcon.value(column);
    else
        return QVariant();
}

QVariant StorageItem::dataAlternate(int column) const
{
    return itemDataAlternate.value(column);
}

void StorageItem::setIcon(int column, const QIcon &icon) 
{
    if( column < itemDataIcon.size() )  // don't set icons for columns past the end
        itemDataIcon[column] = icon;
}

StorageItem *StorageItem::parent() 
{
    return parentItem;
}

