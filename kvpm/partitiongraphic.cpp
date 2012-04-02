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



// Two classes are set defined. Graphic body is privately used by PartitionGraphic
// PartitionGraphic exists only to put a nice frame around the body widget.

class GraphicBody : public QWidget
{
    
public:
    GraphicBody(QWidget *parent = NULL);
    long long  m_preceding_sectors;
    long long  m_following_sectors;
    long long  m_partition_sectors;
    
    void paintEvent(QPaintEvent *);
};

GraphicBody::GraphicBody(QWidget *parent) : QWidget(parent)
{
    m_preceding_sectors = 0;
    m_following_sectors = 0;
    m_partition_sectors = 1;
}

void GraphicBody::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(Qt::blue);
    
    const long double total_sectors = m_preceding_sectors + m_following_sectors + m_partition_sectors;
    double offset = 0;
    double length = (m_preceding_sectors / total_sectors) * width();
    QRectF preceding_rectangle(offset, 0.0, length, (double)height());
    
    offset += length;
    length = (m_partition_sectors / total_sectors) * width();
    if (length < 1.0)                                      // always show at least a sliver
        length = 1.0;
    QRectF partition_rectangle(offset, 0.0, length, (double)height());
    
    offset += length;
    length = (m_following_sectors / total_sectors) * width();
    QRectF following_rectangle(offset, 0.0, length, (double)height());
    
    QBrush free_brush(Qt::green, Qt::SolidPattern);
    QBrush partition_brush(Qt::blue, Qt::SolidPattern);
    
    painter.fillRect(preceding_rectangle, free_brush);
    painter.fillRect(following_rectangle, free_brush);
    painter.fillRect(partition_rectangle, partition_brush);
}



PartitionGraphic::PartitionGraphic(QWidget *parent) : QFrame(parent)
{
    setFixedWidth(250);
    setMinimumHeight(30);

    QVBoxLayout *const layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setMargin(0);
    setFrameStyle(QFrame::Sunken | QFrame::Panel);
    setLineWidth(2);
    setLayout(layout);

    m_body = new GraphicBody();
    layout->addWidget(m_body);
}

void PartitionGraphic::setPrecedingSectors(const long long precedingSectors)
{
    m_body->m_preceding_sectors = precedingSectors;
}

void PartitionGraphic::setFollowingSectors(const long long followingSectors)
{
    m_body->m_following_sectors = followingSectors;
}

void PartitionGraphic::setPartitionSectors(const long long partitionSectors)
{
    m_body->m_partition_sectors = partitionSectors;
}


