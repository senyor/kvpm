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

#ifndef PVPROPERTIESSTACK_H
#define PVPROPERTIESSTACK_H

#include <QStackedWidget>
#include <QTreeWidget>
#include <QList>
 
class PhysVol;

class PVPropertiesStack : public QStackedWidget
{
Q_OBJECT
    bool m_is_pv;
    VolGroup *m_vg;
    QList<QStackedWidget *> m_pv_stack_list;
    
 public:
    PVPropertiesStack(VolGroup *volumeGroup, QWidget *parent = 0);
 
 public slots:
    void changePVStackIndex(QTreeWidgetItem *item, QTreeWidgetItem*);
    
};

#endif
