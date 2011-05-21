/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include <KPushButton>

#include <QtGui>

#include "misc.h"


NoMungeCheck::NoMungeCheck(const QString text, QWidget *parent) : QCheckBox(text, parent)
{
    m_unmunged_text = text;
}

QString NoMungeCheck::getAlternateText()
{
    return m_alternate_text;
}

QStringList NoMungeCheck::getAlternateTextList()
{
    return m_alternate_text_list;
}

QString NoMungeCheck::getUnmungedText()
{
    return m_unmunged_text;
}

void NoMungeCheck::setAlternateText(QString alternateText)
{
    m_alternate_text = alternateText;
}

void NoMungeCheck::setAlternateTextList(QStringList alternateTextList)
{
    m_alternate_text_list = alternateTextList;
}

void NoMungeCheck::setData(QVariant data)
{
    m_data = data;
}

QVariant NoMungeCheck::getData()
{
    return m_data;
}

NoMungeRadioButton::NoMungeRadioButton(const QString text, QWidget *parent) : QRadioButton(text, parent)
{
    m_unmunged_text = text;
}

QString NoMungeRadioButton::getAlternateText()
{
    return m_alternate_text;
}

QString NoMungeRadioButton::getUnmungedText()
{
    return m_unmunged_text;
}

void NoMungeRadioButton::setAlternateText(QString alternateText)
{
    m_alternate_text = alternateText;
}

void NoMungeRadioButton::setData(QVariant data)
{
    m_data = data;
}

QVariant NoMungeRadioButton::getData()
{
    return m_data;
}


/* The idea here is to not have numbers over three digits long
   before the decimal point. This will need to be worked on
   to optionally provide proper SI units in the future */

QString sizeToString(long long bytes)
{
    double size = (double)bytes; 

    if(size < 1000)
	return QString("%1").arg(bytes);

    if( (size /= 1024) < 1000)
	return QString("%1 KiB").arg(size, 0,'g', 3);

    if( (size /= 1024) < 1000)
	return QString("%1 MiB").arg(size, 0, 'g', 3);

    if( (size /= 1024) < 1000)
	return QString("%1 GiB").arg(size, 0,'g', 3);

    size /= 1024;

    return QString("%1 TB").arg(size, 0, 'g', 3);
}


