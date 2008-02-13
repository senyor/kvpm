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
    unmunged_text = text;
}

QString NoMungeCheck::getUnmungedText()
{
    return unmunged_text;
}

