/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
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

#include <QStringList>
#include <QCheckBox>
#include <QRadioButton>
#include <QVariant>

QString sizeToString(long long bytes);
QStringList splitUuid(QString const uuid);
bool isBusy(const QString device);


class NoMungeCheck : public QCheckBox
{
    QString     m_unmunged_text;         // QCheckBox text() without amperands
    QString     m_alternate_text;        // We can put anything we want in here
    QStringList m_alternate_text_list;   // Ditto
    QVariant    m_data;

 public:
    explicit NoMungeCheck(const QString text, QWidget *parent = NULL);
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
    explicit NoMungeRadioButton(const QString text, QWidget *parent = NULL);
    QString getAlternateText();
    QString getUnmungedText();
    void setAlternateText(QString alternateText);
    void setData(QVariant data);
    QVariant getData();    

};

#endif
