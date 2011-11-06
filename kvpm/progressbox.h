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


#ifndef PROGRESSBOX_H
#define PROGRESSBOX_H

#include <QFrame>
#include <QLabel>
#include <QProgressBar>
#include <QString>


class ProgressBox : public QFrame
{
    QLabel       *m_message;
    QProgressBar *m_progressbar;

 public:
    explicit ProgressBox(QWidget *parent = NULL);
    void reset();
    void setText(const QString text);
    void setRange(const int start, const int end);
    void setValue(const int value);
};

#endif
