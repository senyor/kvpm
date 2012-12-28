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


#ifndef PARTITIONGRAPHIC_H
#define PARTITIONGRAPHIC_H


#include <QLabel>
#include <QGroupBox>

class GraphicBody;


class PartitionGraphic : public QGroupBox
{
    Q_OBJECT

    bool m_use_si_units;

    GraphicBody *m_body;
    long long m_total_space;

    QLabel *m_preceding_label,
           *m_following_label,
           *m_change_label,
           *m_move_label;

public:
    PartitionGraphic(const long long space, const bool isNewPart,QWidget *parent = 0);
    void update(long long size, long long offset, const long long move, const long long change);
    void update(const long long size, const long long offset);

};

#endif
