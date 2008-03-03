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


#ifndef EXECUTABLEFINDER_H
#define EXECUTABLEFINDER_H

#include <QObject>
#include <QMap>
#include <QString>

class ExecutableFinder : public QObject
{

    QMap<QString, QString> m_path_map;
    
 public:
    ExecutableFinder(QObject *parent = 0);
    QString getExecutablePath(QString name);
    
};

#endif
