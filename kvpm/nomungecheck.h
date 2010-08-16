/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef NOMUNGECHECK_H
#define NOMUNGECHECK_H

#include <QString>
#include <QCheckBox>


class NoMungeCheck : public QCheckBox
{
    QString m_unmunged_text;    // QCheckBox text() without amperands
    QString m_alternate_text;   // We can put anything we want in here
 
 public:
    NoMungeCheck(const QString text, QWidget *parent = 0);
    QString getAlternateText();
    QString getUnmungedText();
    void setAlternateText(QString alternateText);
    
};

#endif