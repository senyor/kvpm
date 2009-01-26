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

#ifndef TABLECREATE_H
#define TABLECREATE_H

#include <KDialog>

#include <QRadioButton>
#include <QString>

bool create_table(QString devicePath);


class TableCreateDialog : public KDialog
{
Q_OBJECT

    QString   m_device_path;


    QRadioButton *m_msdos_button, 
                 *m_gpt_button;
    
 public:
    TableCreateDialog(QString devicePath, QWidget *parent = 0);
    
 private slots:
    void commitTable();
    
};

#endif
