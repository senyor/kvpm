/*
 *
 * 
 * Copyright (C) 2008, 2010 Benjamin Scott   <benscott@nwlink.com>
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

#include <QString>
#include <QCheckBox>
#include <QRadioButton>
#include <QVariant>

QString sizeToString(long long bytes);

struct AvailableDevice {
    QString name;
    long long size;
};

class NoMungeCheck : public QCheckBox
{
    QString m_unmunged_text;    // QCheckBox text() without amperands
    QString m_alternate_text;   // We can put anything we want in here
    QVariant m_data;

 public:
    NoMungeCheck(const QString text, QWidget *parent = 0);
    QString getAlternateText();
    QString getUnmungedText();
    void setAlternateText(QString alternateText);
    void setData(QVariant data);
    QVariant getData();    
};

class NoMungeRadioButton : public QRadioButton
{
    QString m_unmunged_text;    // QCheckBox text() without amperands
    QString m_alternate_text;   // We can put anything we want in here
    QVariant m_data;

 public:
    NoMungeRadioButton(const QString text, QWidget *parent = 0);
    QString getAlternateText();
    QString getUnmungedText();
    void setAlternateText(QString alternateText);
    void setData(QVariant data);
    QVariant getData();    
};

#endif
