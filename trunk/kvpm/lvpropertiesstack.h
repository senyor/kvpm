/*
 *
 * 
 * Copyright (C) 2008, 2010 Benjamin Scott   <benscott@nwlink.com>
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

    bool m_is_vg;
    VolGroup *m_vg;
    QList<QStackedWidget *> m_lv_stack_list;
    
 public:
    explicit LVPropertiesStack(VolGroup *Group, QWidget *parent = 0);
 
 public slots:
    void changeLVStackIndex(QTreeWidgetItem *item, QTreeWidgetItem*);
    
};

#endif
