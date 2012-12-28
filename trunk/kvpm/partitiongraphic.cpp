/*
 *
 *
 * Copyright (C) 2009, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "partitiongraphic.h"

#include <QDebug>
#include <QPainter>
#include <QVBoxLayout>

#include <KConfigSkeleton>
#include <KGlobal>
#include <KLocale>


// Two classes are set defined. Graphic body is privately used by PartitionGraphic
// PartitionGraphic exists only to put a nice frame around the body widget.

class GraphicBody : public QWidget
{
    
public:
    GraphicBody(QWidget *parent = NULL);
    long long  m_preceding;
    long long  m_following;
    long long  m_size;
    
    void paintEvent(QPaintEvent *);
};

GraphicBody::GraphicBody(QWidget *parent) : QWidget(parent)
{
    setMinimumWidth(250);
    setMinimumHeight(30);

    m_preceding = 0;
    m_following = 0;
    m_size = 1;
}

void GraphicBody::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(Qt::blue);
    
    const long double total = m_preceding + m_following + m_size;
    double offset = 0;
    double length = (m_preceding / total) * width();
    QRectF preceding_rectangle(offset, 0.0, length, (double)height());
    
    offset += length;
    length = (m_size / total) * width();
    if (length < 1.0)                                      // always show at least a sliver
        length = 1.0;
    QRectF partition_rectangle(offset, 0.0, length, (double)height());
    
    offset += length;
    length = (m_following / total) * width();
    QRectF following_rectangle(offset, 0.0, length, (double)height());
    
    QBrush free_brush(Qt::green, Qt::SolidPattern);
    QBrush partition_brush(Qt::blue, Qt::SolidPattern);
    
    painter.fillRect(preceding_rectangle, free_brush);
    painter.fillRect(following_rectangle, free_brush);
    painter.fillRect(partition_rectangle, partition_brush);
}



PartitionGraphic::PartitionGraphic(long long space, bool isNewPart, QWidget *parent) 
    : QGroupBox(parent),
      m_total_space(space)
{
    if (m_total_space < 0)
        m_total_space = 0;

    KConfigSkeleton skeleton;
    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", m_use_si_units, false);

    QVBoxLayout *const layout = new QVBoxLayout();

    QFrame *const graphic_frame = new QFrame();
    QVBoxLayout *const graphic_layout = new QVBoxLayout();
    graphic_frame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    graphic_frame->setLineWidth(2);
    graphic_layout->setSpacing(0);
    graphic_layout->setMargin(0);
    graphic_frame->setLayout(graphic_layout);

    m_body = new GraphicBody();
    graphic_layout->addWidget(m_body);
    layout->addWidget(graphic_frame);

    QHBoxLayout *const lower_layout = new QHBoxLayout();
    QVBoxLayout *const info_layout = new QVBoxLayout();
    m_preceding_label = new QLabel();
    m_following_label = new QLabel();
    m_change_label = new QLabel();
    m_move_label = new QLabel();
    info_layout->addSpacing(5);
    info_layout->addWidget(m_preceding_label);
    info_layout->addWidget(m_following_label);
    info_layout->addSpacing(5);
    info_layout->addWidget(m_change_label);
    info_layout->addWidget(m_move_label);

    if (isNewPart) {
        m_change_label->hide();
        m_move_label->hide();
    }

    info_layout->addSpacing(5);
    lower_layout->addLayout(info_layout);
    lower_layout->addStretch();
    layout->addLayout(lower_layout);

    setLayout(layout);
}

void PartitionGraphic::update(long long size, long long offset, long long  move, long long change)
{
    if (size < 0)
        size = 0;

    if (offset < 0)
        offset = 0;

    long long following = m_total_space - (size + offset);

    if (following < 0)
        following = 0;
    
    m_body->m_preceding = offset;
    m_body->m_following = following;
    m_body->m_size = size;
    m_body->repaint();

    KLocale::BinaryUnitDialect dialect;
    KLocale *const locale = KGlobal::locale();

    if (m_use_si_units)
        dialect = KLocale::MetricBinaryDialect;
    else
        dialect = KLocale::IECBinaryDialect;

    if (change < 0)
        m_change_label->setText(i18n("Reduce size: -%1", locale->formatByteSize(qAbs(change), 1, dialect)));
    else if (change == 0)
        m_change_label->setText(i18n("Size: no change"));
    else
        m_change_label->setText(i18n("Extend size: %1", locale->formatByteSize(change, 1, dialect)));
    
    if (move < 0)
        m_move_label->setText(i18n("Move (left): -%1", locale->formatByteSize(qAbs(move), 1, dialect)));
    else if (move == 0)
        m_move_label->setText(i18n("Move: no change"));
    else
        m_move_label->setText(i18n("Move (right): %1", locale->formatByteSize(move, 1, dialect)));

    m_preceding_label->setText(i18n("Preceding free space: %1", locale->formatByteSize(offset, 1, dialect)));
    m_following_label->setText(i18n("Following free space: %1", locale->formatByteSize(following, 1, dialect)));
}

void PartitionGraphic::update(long long size, long long offset)
{
    update(size, offset, 0, 0);
}

