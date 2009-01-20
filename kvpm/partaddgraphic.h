/*
 *
 * 
 * Copyright (C) 2009 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef PARTADDGRAPHIC_H
#define PARTADDGRAPHIC_H

#include <QFrame>


class PartAddGraphic : public QFrame
{
Q_OBJECT

    long long  m_preceding_sectors;
    long long  m_following_sectors;
    long long  m_partition_sectors;

    void paintEvent(QPaintEvent *);

public:
    PartAddGraphic(QWidget *parent = 0);
    void setPrecedingSectors(long long precedingSectors);
    void setFollowingSectors(long long followingSectors);
    void setPartitionSectors(long long partitionSectors);

};

#endif
