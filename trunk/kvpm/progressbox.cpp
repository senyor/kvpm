/*
 *
 *
 * Copyright (C) 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "progressbox.h"

#include <KApplication>

#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QString>
#include <QVBoxLayout>


ProgressBox::ProgressBox(QWidget *parent) : QFrame(parent)
{
    QHBoxLayout *layout = new QHBoxLayout();
    m_message = new QLabel();
    m_message->setMinimumWidth(150);
    m_message->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_progressbar = new QProgressBar();
    m_progressbar->setTextVisible(false);
    layout->setMargin(0);
    layout->addWidget(m_message);
    layout->addWidget(m_progressbar);
    setLayout(layout);
}

void ProgressBox::setText(const QString text)
{
    hide();

    if (!text.isEmpty())
        m_message->setText(text + " >>");
    else
        m_message->clear();

    show();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

void ProgressBox::setRange(const int start, const int end)
{
    hide();

    m_progressbar->setRange(start, end);
    m_message->clear();

    show();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

}

void ProgressBox::setValue(const int value)
{
    hide();

    m_progressbar->setValue(value);
    if (value >= m_progressbar->maximum())
        m_message->clear();

    show();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

void ProgressBox::reset()
{
    hide();

    m_progressbar->setRange(0, 1);
    m_progressbar->setValue(1);
    m_message->clear();

    show();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}
