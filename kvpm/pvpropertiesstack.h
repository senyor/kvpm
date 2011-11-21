/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
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

#include <QFrame>
#include <QLabel>
#include <QList>
#include <QStackedWidget>
#include <QTreeWidgetItem>
#include <QScrollArea>

 
class PhysVol;
class VolGroup;

class PVPropertiesStack : public QFrame
{
Q_OBJECT

    QStackedWidget *m_stack_widget;
    bool m_is_pv;
    VolGroup *m_vg;
    QList<QStackedWidget *> m_pv_stack_list;
    QLabel *m_pv_label;       // The name of the device
    QScrollArea *m_vscroll;    

 public:
    explicit PVPropertiesStack(VolGroup *volumeGroup, QWidget *parent = 0);
    void loadData();
 
 public slots:
    void changePVStackIndex(QTreeWidgetItem *item, QTreeWidgetItem*);
    
};

#endif
