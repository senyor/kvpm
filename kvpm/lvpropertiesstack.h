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

#ifndef LVPROPERTIESSTACK_H
#define LVPROPERTIESSTACK_H

#include <QFrame>
#include <QLabel>
#include <QList>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTreeWidget>
 
class VolGroup;


class LVPropertiesStack : public QFrame
{
Q_OBJECT

    VolGroup *m_vg;
    QStackedWidget *m_stack_widget;
    QList<QStackedWidget *> m_lv_stack_list;
    QScrollArea *m_vscroll;
    QLabel *m_lv_label;
    
 public:
    explicit LVPropertiesStack(VolGroup *Group, QWidget *parent = 0);
    void loadData();

 public slots:
    void changeLVStackIndex(QTreeWidgetItem *item, QTreeWidgetItem*);
    
};

#endif
