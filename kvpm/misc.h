/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef MISC_H
#define MISC_H

#include <QCheckBox>
#include <QFileInfo>
#include <QRadioButton>
#include <QStringList>
#include <QVariant>

#include "typedefs.h"

class PhysVol;
class StorageBase;

typedef enum {
    LINEAR    = 0,
    LVMMIRROR = 1,
    RAID1     = 2,
    RAID4     = 3,
    RAID5     = 4,
    RAID6     = 5,
    THIN      = 6 
} VolumeType;

struct PvSpace {
    PhysVol *pv;
    long long normal;
    long long contiguous;

    PvSpace(PhysVol *volume, long long norm, long long cont) {pv = volume; normal = norm; contiguous = cont;}
};


QStringList splitUuid(QString const uuid);
bool isBusy(const QString device);
QString findMapperPath(QString path);
QList<const StorageBase *> getUsablePvs();


class NoMungeCheck : public QCheckBox
{
    QString     m_unmunged_text;         // QCheckBox text() without amperands
    QString     m_alternate_text;        // We can put anything we want in here
    QStringList m_alternate_text_list;   // Ditto
    QVariant    m_data;

public:
    explicit NoMungeCheck(const QString text, QWidget *parent = nullptr);
    QString     getAlternateText();
    QStringList getAlternateTextList();
    QString     getUnmungedText();
    void setAlternateText(QString alternateText);
    void setAlternateTextList(QStringList alternateTextList);
    void setData(QVariant data);
    QVariant getData();
};

class NoMungeRadioButton : public QRadioButton
{
    QString m_unmunged_text;    // QCheckBox text() without amperands
    QString m_alternate_text;   // We can put anything we want in here
    QVariant m_data;

public:
    explicit NoMungeRadioButton(const QString text, QWidget *parent = nullptr);
    QString getAlternateText();
    QString getUnmungedText();
    void setAlternateText(QString alternateText);
    void setData(QVariant data);
    QVariant getData();

};

#endif
