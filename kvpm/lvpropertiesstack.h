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

#ifndef LVPROPERTIESSTACK_H
#define LVPROPERTIESSTACK_H

#include <QStackedWidget>
#include <QTreeWidget>
#include <QList>
 
class VolGroup;

class LVPropertiesStack : public QStackedWidget
{
Q_OBJECT

    bool is_vg;
    VolGroup *vg;
    QList<QStackedWidget *> lv_stack_list;
    
 public:
    LVPropertiesStack(VolGroup *Group, QWidget *parent = 0);
 
 public slots:
    void changeLVStackIndex(QTreeWidgetItem *item, QTreeWidgetItem*);
    
};

#endif
