/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Klvm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */
#ifndef LVREMOVE_H
#define LVREMOVE_H

#include <QStringList>

class LogVol;

bool remove_lv(LogVol *LogicalVolume);
bool remove_lv(QString FullName);

class LVRemoveDialog : public QObject
{
    QString lv_full_name;   //full name = vg name + lv name
    void buildDialog();
    int return_code;
    
 public:
    LVRemoveDialog(QString FullName, QWidget *parent = 0);
    QStringList arguments();
    int result();
    
};

#endif
