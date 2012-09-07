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

#include "misc.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <KPushButton>


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

QStringList splitUuid(QString const uuid) // Turns a one line uuid into two shorter lines for veiwing
{
    int split_index = 0;

    if (uuid.count('-') < 4) {
        return QStringList(uuid) << "";
    } else if (uuid.count('-') < 5) {
        for (int x = 0; x < 4; x++)
            split_index = uuid.indexOf('-', split_index + 1);
    } else {
        for (int x = 0; x < 5; x++)
            split_index = uuid.indexOf('-', split_index + 1);
    }

    if (split_index <= 0)
        split_index = uuid.size() - 1;

    QString const uuid_start = uuid.left(split_index + 1);
    QString const uuid_end   = uuid.right((uuid.size() - split_index) - 1);

    return QStringList(uuid_start)  << uuid_end;
}

// Checks to see if a device is busy on linux 2.6+
bool isBusy(const QString device)
{
    struct stat     st_buf;
    int             fd;

    QByteArray dev_qba = device.toLocal8Bit();

    if (stat(dev_qba.data(), &st_buf) != 0)
        return false;

    fd = open(dev_qba.data(), O_RDONLY | O_EXCL);
    if (fd < 0) {
        if (errno == EBUSY)
            return true;
    } else
        close(fd);

    return false;
}

QString policyToString(AllocationPolicy policy)
{
    switch(policy) {
    case NO_POLICY:
        return QString("");
        break;
    case NORMAL:
        return QString("normal");
        break;
    case CONTIGUOUS:
        return QString("contiguous");
        break;
    case CLING:
        return QString("cling");
        break;
    case ANYWHERE:
        return QString("anywhere");
        break;
    case INHERITED:
        return QString("inherited");
        break;
    default:
        return QString("normal");
    } 
}
