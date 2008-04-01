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


#include <QtGui>

#include "nomungecheck.h"


NoMungeCheck::NoMungeCheck(const QString text, QWidget *parent) : QCheckBox(text, parent)
{
    m_unmunged_text = text;
}

QString NoMungeCheck::getAlternateText()
{
    return m_alternate_text;
}

QString NoMungeCheck::getUnmungedText()
{
    return m_unmunged_text;
}

void NoMungeCheck::setAlternateText(QString alternateText)
{
    m_alternate_text = alternateText;
}

