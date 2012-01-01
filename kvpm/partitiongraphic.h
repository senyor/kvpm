/*
 *
 * 
 * Copyright (C) 2009, 2011 Benjamin Scott   <benscott@nwlink.com>
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

#include <QFrame>


class GraphicBody;


class PartitionGraphic : public QFrame
{
Q_OBJECT

    GraphicBody  *m_body;

public:
    PartitionGraphic(QWidget *parent = 0);
    void setPrecedingSectors(const long long precedingSectors);
    void setFollowingSectors(const long long followingSectors);
    void setPartitionSectors(const long long partitionSectors);

};

#endif
